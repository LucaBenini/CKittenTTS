Here’s the updated README with your changes applied.

---

# CKitten

**CKitten** is a minimal **C port** of [KittenTTS](https://github.com/KittenML/KittenTTS/), a text-to-speech system.

It now runs on **Windows** and **Linux** (both **x64** and **ARM64**, e.g., Raspberry Pi 64-bit OS).

---

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
