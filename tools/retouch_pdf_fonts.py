import re
from pathlib import Path

import fitz


ROOT = Path(__file__).resolve().parents[1]
FONT = Path(r"C:\Users\HP\AppData\Local\Programs\MiKTeX\fonts\opentype\public\lm\lmroman10-regular.otf")

FIGURES = [
    "Images/realgraph/Varying the update number.pdf",
    "Images/realgraph/vary_initial_set_size_real_pdf.pdf",
    "Images/realgraph/vary_k_real_pdf.pdf",
    "Images/realgraph/legende.pdf_2.pdf",
    "Images/realgraph/dim_hdr_pdf.pdf",
    "Images/realgraph/dim_delta_pdf.pdf",
]

SKIP_TEXT = {
    "Varying the update number.pdf": {"Varying the update number"},
    "vary_initial_set_size_real_pdf.pdf": {"Varying the initial set size"},
    "vary_k_real_pdf.pdf": {"Varying K"},
}


def rewrite_pdf(src: Path) -> Path:
    dst = src.with_name(src.stem + "_lm.pdf")
    doc = fitz.open(src)
    for page in doc:
        spans = []
        for block in page.get_text("dict")["blocks"]:
            for line in block.get("lines", []):
                direction = tuple(round(x, 3) for x in line.get("dir", (1, 0)))
                for span in line.get("spans", []):
                    text = span.get("text", "")
                    if not text.strip():
                        continue
                    spans.append(
                        {
                            "text": text,
                            "origin": fitz.Point(span["origin"]),
                            "bbox": fitz.Rect(span["bbox"]),
                            "size": span["size"],
                            "color": span.get("color", 0),
                            "dir": direction,
                        }
                    )

        normalized = []
        skip_next = False
        for idx, span in enumerate(spans):
            if skip_next:
                skip_next = False
                continue
            text = span["text"]
            if text in SKIP_TEXT.get(src.name, set()):
                continue
            if text == "\u2206" and idx + 1 < len(spans) and spans[idx + 1]["text"] == "-Tree":
                nxt = spans[idx + 1].copy()
                nxt["text"] = "Delta-Tree"
                nxt["origin"] = span["origin"]
                normalized.append(nxt)
                skip_next = True
                continue
            if text == "Varying Pruning Dims on -Tree":
                span = span.copy()
                span["text"] = "Varying Pruning Dims on Delta-Tree"
            normalized.append(span)
        spans = normalized

        for xref in page.get_contents():
            stream = doc.xref_stream(xref)
            if stream:
                stream = re.sub(rb"BT\b.*?\bET", b"", stream, flags=re.S)
                doc.update_stream(xref, stream)

        if src.name == "dim_delta_pdf.pdf":
            page.draw_rect(fitz.Rect(340, 8, 363, 30), color=None, fill=(1, 1, 1), overlay=True)

        page.insert_font(fontname="LMRoman", fontfile=str(FONT))

        for span in spans:
            color_int = span["color"]
            color = (
                ((color_int >> 16) & 255) / 255,
                ((color_int >> 8) & 255) / 255,
                (color_int & 255) / 255,
            )
            rotate = 90 if span["dir"] == (0.0, -1.0) else 0
            page.insert_text(
                span["origin"],
                span["text"],
                fontname="LMRoman",
                fontsize=span["size"],
                color=color,
                rotate=rotate,
            )

    doc.save(dst, garbage=4, deflate=True)
    doc.close()
    return dst


def main():
    for rel in FIGURES:
        src = ROOT / rel
        dst = rewrite_pdf(src)
        print(f"{src.relative_to(ROOT)} -> {dst.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
