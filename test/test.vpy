import vapoursynth as vs
from vapoursynth import core

core.max_cache_size = 4000
source = core.ffms2.Source(r'../test/test.mp4')

def sobel_mask(clip: vs.VideoNode, thr: int = 40) -> vs.VideoNode:
    sob = core.std.ShufflePlanes(clip, 0, vs.GRAY).std.Sobel()
    return sob.std.Expr(f'x {thr} < 0 x ?')

#sobel_mask(source).set_output(0)
source.set_output(0)
