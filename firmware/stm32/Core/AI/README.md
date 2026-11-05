# Generated model code

Everything else in this folder is generated or copied, so never edit it by hand. It's committed so
the project builds without X-CUBE-AI installed; only retraining needs that.

After `python ml/train.py`, run this from the repo root, then copy `network*.c` and `network*.h` from
`<out>`, and `ml/model/gesture_model.h`, into this folder:

```bash
stedgeai generate --model ml/model/gesture_model.onnx --target stm32f4 --compression none --output <out>
```

`Inc/` and `Lib/` are the runtime, copied from the X-CUBE-AI pack's `Middlewares/ST/AI` (the library is
`Lib/GCC/ARMCortexM4/NetworkRuntime1020_CM4_GCC.a`). They only change with a new X-CUBE-AI version, and
then the library's name in `.cproject` changes too.

`stedgeai.exe` is in the pack's `Utilities/windows`. It won't run from a path over 40 characters unless
Windows long paths are on, so run it through a short link to the pack instead:

```powershell
New-Item -ItemType Junction -Path D:\CU\stai -Target "$env:USERPROFILE\STM32Cube\Repository\Packs\STMicroelectronics\X-CUBE-AI\10.2.1"
```
