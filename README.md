# rt

Another exercise in programming...

<hr>

2020-07-23 - Integrated with OpenImageDenoise:
<img src=screens/1595537701-627SD.png /><br>
627 samples, **denoised**

2020-07-22 - New improved BSDF, Russian Roulette ray termination based on path weight, Uncharted 2 filmic tonemapping:
<img src=screens/1595468010-2000S.png /><br>
2000 samples, 0.26632s per sample (1.59s), _I only dislike that shadow in front of the blue box..._

2020-07-20 - Fixed diffuse term in PBR material... It's seems too dark and desaturated now, though. Will need to take some time and *really* dive into the math...
<img src=screens/1595284318-2823S.png /><br>

2020-07-18 - Scene import from Blender using custom JSD file. Some artifacts are visible here - that's the reason why PBR material BSDF needs to be reworked:<br>
<img src=screens/1595029745-893S.png /><br>
~893 samples, 0.5259 per sample (3.15s)

2020-07-13 - Russian roulette for ray extinction, multithreading and other cool stuff:<br>
<img src=screens/2020-07-13-1.png /><br>
~1006 samples, 0.2299s per sample (1.37s per thread / 6), 0.05 ray extinction probability

2020-07-08 - ray transmission and Russian roulette for reflection/transmission sampling:<br>
<img src=screens/2020-07-08-1.png /><br>
~854 samples, 1.888s per sample

2020-07-07 - scene object transform:<br>
<img src=screens/2020-07-07-1.png /><br>
~808 samples, 0.923s per sample

2020-07-04 - optimized and tweaked BVH tree acceleration:<br>
<img src=screens/2020-07-04-1.png /><br>
~3800 samples, 0.795s per sample

2020-07-02 - Model importing, basic BVH acceleration:<br>
<img src=screens/2020-07-02-1.png /><br>
~250 samples, 4.840s per sample

2020-07-01 - Still trying to get PBR right...:<br>
<img src=screens/2020-07-01-2.png /><br>
<img src=screens/with_blender.png /><br>
2000+ samples, 905ms per sample
<img src=screens/2020-07-01-1.png /><br>
~1400 samples, 320.4ms per sample

2020-06-30 - Cook-Torrance materials, fixed sampling and emissive materials handling:<br>
<img src=screens/2020-06-30-2.png /><br>
~1800 samples, 233.8ms per sample

2020-06-30 - Reinhard operator, gamma correction, anti-aliasing and sample averaging:<br>
<img src=screens/2020-06-30-1.png /><br>

2020-06-29 - basic path tracing:<br>
<img src=screens/2020-06-29-2.png /><br>
<img src=screens/2020-06-29-1.png /><br>

2020-06-27/28 - first steps:<br>
<img src=screens/2020-06-28-1.png /><br>
<img src=screens/2020-06-27-1.png /><br>

