#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>

#include "DataBase.h"
#include "HDR_Tree.h"
#include "Item.h"
#include "itemDataBase.h"
#include "itemHDR_Tree.h"

struct BenchConfig {
    std::string dataset = "datasets/Normalized_WT.dat";
    long features = 128;
    long users = 20000;
    long items = 20000;
    long general_num = 60000;
    long general_user = 30000;
    long general_item = 30000;
    long fanout_hdr = 15;
    long threshold_hdr = 5;
    long fanout_delta = 10;
    long threshold_delta = 5;
    long k = 10;
    long divide_num = 13; // for init_half_behind_loop
    std::string mode = "all"; // all | dual | single_hdr | single_delta | em | simplified
    bool quiet = true;
};

struct MemorySnapshot {
    std::uint64_t working_set_bytes = 0;
    std::uint64_t private_bytes = 0;
};

static bool read_process_memory(MemorySnapshot &m) {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(),
                              reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc),
                              sizeof(pmc))) {
        return false;
    }
    m.working_set_bytes = static_cast<std::uint64_t>(pmc.WorkingSetSize);
    m.private_bytes = static_cast<std::uint64_t>(pmc.PrivateUsage);
    return true;
}

static double bytes_to_mb(std::uint64_t b) {
    return static_cast<double>(b) / (1024.0 * 1024.0);
}

static long clamp_positive(long x, long fallback_value) {
    return x > 0 ? x : fallback_value;
}

class ScopedCoutSilence {
public:
    explicit ScopedCoutSilence(bool enabled) : enabled_(enabled) {
        if (enabled_) {
            old_ = std::cout.rdbuf(sink_.rdbuf());
        }
    }
    ~ScopedCoutSilence() {
        if (enabled_) {
            std::cout.rdbuf(old_);
        }
    }
private:
    bool enabled_ = false;
    std::streambuf* old_ = nullptr;
    std::ostringstream sink_;
};

static void configure_item_db(itemDataBase &db, const BenchConfig &cfg) {
    db.verbose = false;
    db.general_num = cfg.general_num;
    db.general_num_user = cfg.general_user;
    db.general_num_item = cfg.general_item;
    db.num_user = cfg.users;
    db.num_item = cfg.items;
    db.random_bit = 0; // deterministic for benchmark reproducibility
}

static void configure_user_db(DataBase &db, const BenchConfig &cfg) {
    db.verbose = false;
    db.general_num = cfg.general_num;
    db.general_num_user = cfg.general_user;
    db.num_user = cfg.users;
    db.num_item = cfg.items;
    db.random_bit = 0; // deterministic for benchmark reproducibility
}

static void prepare_item_matrix(itemDataBase &db, const BenchConfig &cfg) {
    db.load(cfg.dataset);
    db.transformData(cfg.features, false);
    configure_item_db(db, cfg);
    db.generate_general_set();
    db.generate_item_set();
    db.generate_pca_matrix();
    db.computePCA();
}

static void fill_sliding_window(HDR_Tree &tree, const itemDataBase &db, long n_items) {
    tree.slidingWindow.clear();
    const long rows = static_cast<long>(db.I.rows());
    const long count = (std::min)(n_items, rows);
    tree.slidingWindow.reserve(static_cast<size_t>(count));
    for (long i = 0; i < count; ++i) {
        tree.slidingWindow.emplace_back(new Item(db.I.row(i)));
    }
}

static void print_memory_delta(const std::string &tag,
                               const MemorySnapshot &before,
                               const MemorySnapshot &after) {
    std::cout << "[MEM][" << tag << "] "
              << "working_set_delta_mb=" << (bytes_to_mb(after.working_set_bytes) - bytes_to_mb(before.working_set_bytes))
              << ", private_delta_mb=" << (bytes_to_mb(after.private_bytes) - bytes_to_mb(before.private_bytes))
              << std::endl;
}

static void run_single_delta(const BenchConfig &cfg) {
    MemorySnapshot mem_before, mem_after;
    read_process_memory(mem_before);

    auto t0 = std::chrono::steady_clock::now();

    {
        ScopedCoutSilence silence(cfg.quiet);
        itemDataBase idb;
        prepare_item_matrix(idb, cfg);

        itemHDR_Tree delta_tree;
        delta_tree.numFeatures = cfg.features;
        delta_tree.checkDuplicates = false;
        delta_tree.numItems = cfg.items;
        delta_tree.verbose = false;
        delta_tree.setData(&idb);
        delta_tree.construct(cfg.fanout_delta, cfg.threshold_delta);
    }

    auto t1 = std::chrono::steady_clock::now();
    read_process_memory(mem_after);

    const double secs = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;
    std::cout << "[TIME][single_delta_build_sec] " << secs << std::endl;
    print_memory_delta("single_delta", mem_before, mem_after);
}

static void run_single_hdr(const BenchConfig &cfg) {
    MemorySnapshot mem_before, mem_after;
    read_process_memory(mem_before);

    auto t0 = std::chrono::steady_clock::now();

    {
        ScopedCoutSilence silence(cfg.quiet);
        itemDataBase idb;
        prepare_item_matrix(idb, cfg);

        DataBase udb;
        udb.load(cfg.dataset);
        configure_user_db(udb, cfg);

        HDR_Tree hdr_tree;
        hdr_tree.k = cfg.k;
        hdr_tree.windowSize = cfg.items;
        hdr_tree.numFeatures = cfg.features;
        hdr_tree.checkDuplicates = false;
        hdr_tree.numUsers = cfg.users;
        hdr_tree.verbose = false;
        hdr_tree.setData(&udb);
        fill_sliding_window(hdr_tree, idb, cfg.items);
        hdr_tree.construct(cfg.fanout_hdr, cfg.threshold_hdr);
    }

    auto t1 = std::chrono::steady_clock::now();
    read_process_memory(mem_after);

    const double secs = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;
    std::cout << "[TIME][single_hdr_build_sec] " << secs << std::endl;
    print_memory_delta("single_hdr", mem_before, mem_after);
}

static void run_dual(const BenchConfig &cfg, bool run_em, bool run_simplified) {
    MemorySnapshot mem_before, mem_after_build;
    read_process_memory(mem_before);

    double build_secs = 0.0;
    double em_secs = -1.0;
    double simp_secs = -1.0;
    long best_d = -1;

    {
        ScopedCoutSilence silence(cfg.quiet);

        itemDataBase idb;
        idb.load(cfg.dataset);
        configure_item_db(idb, cfg);

        DataBase udb;
        udb.load(cfg.dataset);
        configure_user_db(udb, cfg);

        HDR_Tree dual_tree;
        dual_tree.k = cfg.k;
        dual_tree.windowSize = cfg.items;
        dual_tree.numFeatures = cfg.features;
        dual_tree.checkDuplicates = false;
        dual_tree.numUsers = cfg.users;
        dual_tree.verbose = false;
        dual_tree.setData_i(&idb);
        dual_tree.setData(&udb);

        auto t0 = std::chrono::steady_clock::now();
        dual_tree.numFeatures_i = cfg.features;
        dual_tree.checkDuplicates_i = false;
        dual_tree.numItems_i = cfg.items;
        dual_tree.verbose_i = false;
        dual_tree.construct_i(cfg.fanout_delta, cfg.threshold_delta);
        dual_tree.construct(cfg.fanout_hdr, cfg.threshold_hdr);
        auto t1 = std::chrono::steady_clock::now();
        build_secs = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1e6;
        read_process_memory(mem_after_build);

        if (run_em) {
            auto em_t0 = std::chrono::steady_clock::now();
            dual_tree.init_half_behind_loop(cfg.divide_num);
            auto em_t1 = std::chrono::steady_clock::now();
            em_secs = std::chrono::duration_cast<std::chrono::microseconds>(em_t1 - em_t0).count() / 1e6;
        }

        if (run_simplified) {
            // Simple closed-form scan over sampled candidates:
            // cost(d) = D - D * Pp(d) + d, with Pp(d) approximated from (p1, p2).
            auto s_t0 = std::chrono::steady_clock::now();

            double best_cost = std::numeric_limits<double>::max();
            const long D = cfg.features;
            const size_t n1 = (std::min)(dual_tree.v_d_prune.size(), dual_tree.p1_for_dimensions.size());
            const size_t n = (std::min)(n1, dual_tree.p2_for_dimensions.size());

            for (size_t i = 0; i < n; ++i) {
                const double p1 = dual_tree.p1_for_dimensions[i];
                const double p2 = dual_tree.p2_for_dimensions[i];
                double pp = p1 + (1.0 - p1) * p2;
                pp = (std::max)(0.0, (std::min)(1.0, pp));
                const long d = dual_tree.v_d_prune[i];
                const double cost = static_cast<double>(D) - static_cast<double>(D) * pp + static_cast<double>(d);
                if (cost < best_cost) {
                    best_cost = cost;
                    best_d = d;
                }
            }

            auto s_t1 = std::chrono::steady_clock::now();
            simp_secs = std::chrono::duration_cast<std::chrono::microseconds>(s_t1 - s_t0).count() / 1e6;
        }
    }

    std::cout << "[TIME][dual_build_sec] " << build_secs << std::endl;
    print_memory_delta("dual_build", mem_before, mem_after_build);
    if (run_em) {
        std::cout << "[TIME][em_proxy_sec] " << em_secs << std::endl;
    }
    if (run_simplified) {
        std::cout << "[TIME][simplified_dim_solver_sec] " << simp_secs << std::endl;
        std::cout << "[RESULT][simplified_dim_solver_best_d] " << best_d << std::endl;
    }
}

static BenchConfig parse_args(int argc, char **argv) {
    BenchConfig cfg;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        const auto read_next = [&](long &v) {
            if (i + 1 < argc) v = std::stol(argv[++i]);
        };
        if (a == "--dataset" && i + 1 < argc) cfg.dataset = argv[++i];
        else if (a == "--mode" && i + 1 < argc) cfg.mode = argv[++i];
        else if (a == "--features") read_next(cfg.features);
        else if (a == "--users") read_next(cfg.users);
        else if (a == "--items") read_next(cfg.items);
        else if (a == "--general") read_next(cfg.general_num);
        else if (a == "--general-user") read_next(cfg.general_user);
        else if (a == "--general-item") read_next(cfg.general_item);
        else if (a == "--fanout-hdr") read_next(cfg.fanout_hdr);
        else if (a == "--threshold-hdr") read_next(cfg.threshold_hdr);
        else if (a == "--fanout-delta") read_next(cfg.fanout_delta);
        else if (a == "--threshold-delta") read_next(cfg.threshold_delta);
        else if (a == "--k") read_next(cfg.k);
        else if (a == "--divide-num") read_next(cfg.divide_num);
        else if (a == "--quiet" && i + 1 < argc) cfg.quiet = (std::stol(argv[++i]) != 0);
    }
    cfg.features = clamp_positive(cfg.features, 128);
    cfg.users = clamp_positive(cfg.users, 20000);
    cfg.items = clamp_positive(cfg.items, 20000);
    cfg.general_num = clamp_positive(cfg.general_num, 60000);
    cfg.general_user = clamp_positive(cfg.general_user, 30000);
    cfg.general_item = clamp_positive(cfg.general_item, 30000);
    cfg.fanout_hdr = clamp_positive(cfg.fanout_hdr, 15);
    cfg.threshold_hdr = clamp_positive(cfg.threshold_hdr, 5);
    cfg.fanout_delta = clamp_positive(cfg.fanout_delta, 10);
    cfg.threshold_delta = clamp_positive(cfg.threshold_delta, 5);
    cfg.k = clamp_positive(cfg.k, 10);
    cfg.divide_num = clamp_positive(cfg.divide_num, 13);
    return cfg;
}

int main(int argc, char **argv) {
    BenchConfig cfg = parse_args(argc, argv);

    std::cout << "[CONFIG]"
              << " mode=" << cfg.mode
              << " dataset=" << cfg.dataset
              << " features=" << cfg.features
              << " users=" << cfg.users
              << " items=" << cfg.items
              << std::endl;

    if (cfg.mode == "single_delta") {
        run_single_delta(cfg);
        return 0;
    }
    if (cfg.mode == "single_hdr") {
        run_single_hdr(cfg);
        return 0;
    }
    if (cfg.mode == "dual") {
        run_dual(cfg, false, false);
        return 0;
    }
    if (cfg.mode == "em") {
        run_dual(cfg, true, false);
        return 0;
    }
    if (cfg.mode == "simplified") {
        run_dual(cfg, true, true);
        return 0;
    }

    // all
    run_single_delta(cfg);
    run_single_hdr(cfg);
    run_dual(cfg, true, true);
    return 0;
}
