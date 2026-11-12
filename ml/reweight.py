"""Put a retrained model into the X-CUBE-AI files the firmware builds from.

    python ml/train.py && python ml/reweight.py

X-CUBE-AI generated firmware/stm32/Core/AI/ from an earlier gesture_model.onnx. Retraining on new
recordings changes only the weights and the threshold, not the graph, so the ST tools are not
needed again: this rewrites the weights array in place and copies the header across.

The array holds the raw float32 tensors in initializer order, with conv kernels transposed to
(out, kernel, in). That was established by rebuilding the committed array from the ONNX it was
generated from, byte for byte, which this script repeats as a check on every run.

shortcut: weights and threshold only, run stedgeai generate if the layers in train.py change
"""
import re
from pathlib import Path

import numpy as np
import onnx
from onnx import numpy_helper

ROOT = Path(__file__).resolve().parent.parent
ONNX = ROOT / "ml" / "model" / "gesture_model.onnx"
HEADER = ONNX.with_suffix(".h")
AI = ROOT / "firmware" / "stm32" / "Core" / "AI"
ARRAY = re.compile(r"(s_network_weights_array_u64\[(\d+)\]\s*=\s*\{\n)(.*?)(\};)", re.S)


def weights(path):
    """The model's tensors in the order and layout the generated array stores them."""
    out = []
    for tensor in onnx.load(path).graph.initializer:
        a = numpy_helper.to_array(tensor)
        if a.ndim == 3:  # conv kernel: (out, in, kernel) -> (out, kernel, in)
            a = a.transpose(0, 2, 1)
        out.append(np.ascontiguousarray(a).astype("<f4").tobytes())
    return b"".join(out)


def stored(src):
    """The bytes the array in network_data_params.c currently holds."""
    words = re.findall(r"0x([0-9a-fA-F]{16})U", ARRAY.search(src).group(3))
    return b"".join(bytes.fromhex(w)[::-1] for w in words)


def read(path):
    """The file as it is on disk, with its line endings left alone."""
    with path.open(newline="") as f:
        return f.read()


def render(data):
    words = [f"0x{int.from_bytes(data[i:i + 8], 'little'):016x}U" for i in range(0, len(data), 8)]
    return "".join("  " + " ".join(w + "," for w in words[i:i + 4]) + "\n"
                   for i in range(0, len(words), 4))


def main():
    params = AI / "network_data_params.c"
    src = read(params)
    data = weights(ONNX)
    # the array was sized by the generated graph, so a different length means the layers changed
    if len(data) != len(stored(src)):
        raise SystemExit(f"{ONNX.name} needs {len(data)} bytes, the generated array holds "
                         f"{len(stored(src))}: regenerate Core/AI with stedgeai instead")

    params.write_text(ARRAY.sub(lambda m: m.group(1) + render(data) + m.group(4), src, count=1),
                      newline="\n")
    assert stored(read(params)) == data, "the array written back does not match"
    (AI / "gesture_model.h").write_text(HEADER.read_text(), newline="\n")
    print(f"wrote {params.relative_to(ROOT)} and {(AI / 'gesture_model.h').relative_to(ROOT)}")


if __name__ == "__main__":
    main()
