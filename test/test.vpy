import vapoursynth as vs
from vapoursynth import core

source = core.std.BlankClip(width=1280, height=720, color=(40, 180, 0), format=vs.YUV420P8, length=1000, fpsnum=24000, fpsden=1001)
source = source.text.CoreInfo()
source.set_output(0)