# CKitten

**CKitten** is a minimal **C port** of [KittenTTS](https://github.com/KittenML/KittenTTS/), a text-to-speech system.
This port currently works **only on Windows**, with **hardcoded voice and message**.

It requires **espeak-ng** to be installed and uses it to synthesize speech.
THIS README has been fully ai written. Don't trust it blindly.

---

## 📦 Requirements

1. **Windows** (tested only on Windows so far)
2. **espeak-ng 1.52.0**

   * Download and install the `.msi` from:
     [espeak-ng 1.52.0 release page](https://github.com/espeak-ng/espeak-ng/releases/tag/1.52.0)

⚠ **Important:** CKitten currently assumes that `espeak-ng` is installed and available in your system path.

---

## 🔨 Build

To compile:

```powershell
nmake -f makefile
```

This will produce an executable (`ckitten.exe`).

---

## ▶ Run

At the moment:

* **Voice** is hardcoded (change it in the source to point to an approprate ben if you want another voice).
* **Message** is also hardcoded (edit in the code to change the text to speak).

Run:

```powershell
cd bin
kittentts.exe
```

it will generate/overwrite audio1.wav speaking the hardcoded text.

---

## 🗒 Notes

* The project is in **early stage** — no runtime voice or text selection yet.
* Future plans include:

  * No Plan this is a to migrate to C, next pla
---

## 📜 License

CKitten is a port of KittenTTS source code and follows the same license as the original project. See [KittenTTS LICENSE](https://github.com/KittenML/KittenTTS/blob/main/LICENSE). Or is probably GPLv3. 
