# CKitten

**CKitten** is a lightweight **C port** of the inference engine from [KittenTTS](https://github.com/KittenML/KittenTTS/), a text-to-speech system.

It runs on **Windows** and **Linux**, supporting both **x64** and **ARM64** architectures (e.g., Raspberry Pi 64-bit OS).

Unlike the official Python inference implementation, this port avoids several pitfalls—most notably in the **tokenizer**.  
By removing the inefficiencies of the Python tokenizer wrapper, we achieve significant performance improvements:

| Model | CKitten | Python |
|-------|---------|--------|
| 0.1   | 3.28 s  | 6.31 s |
| 0.2   | 3.55 s  | 7.08 s |

---

### Benchmark (Raspberry Pi)

**Test phrase:**

> "The quick brown fox jumps over the lazy dog. Dr. Smith asked whether it's 3:30 PM today."

#### Python benchmark code
```python
from kittentts import KittenTTS
import time
import soundfile as sf

m = KittenTTS("KittenML/kitten-tts-nano-0.2")

for x in range(0, 11):
    s = time.time()
    audio = m.generate(
        "The quick brown fox jumps over the lazy dog. Dr. Smith asked whether it's 3:30 PM today.",
        voice="expr-voice-2-f"
    )
    e = time.time()
    if x != 0:  # skip first run (warm-up)
        print(f"[{x}] Elapsed time: {e - s:.2f} s")

# Available voices:
# ['expr-voice-2-m', 'expr-voice-2-f', 'expr-voice-3-m', 'expr-voice-3-f',
#  'expr-voice-4-m', 'expr-voice-4-f', 'expr-voice-5-m', 'expr-voice-5-f']

# Save the audio
sf.write("output.wav", audio, 24000)
````

---

⚠️ **Note:**
The Python implementation relies on a phonemizer wrapper that accesses the disk on every call and a loading for the voices.
This introduces additional variability in benchmarking results.



## 📦 Requirements

### Windows

1. **Windows** (tested on recent versions)
2. **espeak-ng 1.52.0**

   * Download and install the `.msi` from the official release page:
     [espeak-ng 1.52.0 release page](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0)

> ⚠️ CKitten assumes `espeak-ng` is installed and available in your system PATH.

---

### Linux (x64 & ARM64, including Raspberry Pi 64-bit)

You’ll need the `espeak-ng` development package, plus a **recent** `onnxruntime`. Many distro packages ship an older `onnxruntime`, so fetch the shared libraries from the Python package and install them system-wide:

```bash
sudo apt install -y libespeak-ng-dev

mkdir /tmp/pv
cd /tmp/pv/
python -mvenv venv
source venv/bin/activate
pip install onnxruntime
sudo cp ./venv/lib/python3.11/site-packages/onnxruntime/capi/libonnxruntime_providers_shared.so /usr/lib
sudo cp ./venv/lib/python3.11/site-packages/onnxruntime/capi/libonnxruntime.so.1.22.1 /usr/lib/libonnxruntime.so
sudo ldconfig
```

> 💡 If your system Python version differs from `3.11`, adjust the paths accordingly in the `cp` commands.

---

## 🔨 Build

**Windows (Developer Command Prompt):**

```powershell
nmake -f makefile.win
```

**Linux (GNU Make):**

```bash
make -f makefile.lin
```

This will produce the executable in `bin/`:

* `bin/kittentts.exe` on Windows
* `bin/kittentts` on Linux

---

## ▶️ Usage

```
./kittentts <model_path> <voice_path> <output> <message> <speed>

  model_path : Path to the model file
  voice_path : Path to the voice file
  output     : Wav file to create
  message    : Text to synthesize
  speed      : Speech speed (float)
```

**Example:**

```bash
./kittentts kitten_tts_nano_v0_1.onnx expr-voice-2-m.bin sample.wav \
"Kitten TTS is an open-source series of tiny and expressive Text-to-Speech models" 1.0
```

---

## 🗒 Notes

* Supports **Windows** and **Linux** (x64 & ARM64).
* Runtime selection of **model**, **voice**, **output file**, **text**, and **speed** is supported via CLI arguments.

---

## 📜 License

CKitten is a port of KittenTTS source code and follows the same license as the original project. See the [KittenTTS LICENSE](https://github.com/KittenML/KittenTTS/blob/main/LICENSE).
