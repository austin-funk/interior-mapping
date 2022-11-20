# Projects 5 & 6: Lights, Camera & Action!

All project handouts can be found [here](https://cs1230.graphics/projects).

PROJECT 5:
Overall a fun project, didn't take too long but was forced to start late due to unforeseen circumstances. There
aren't any known bugs as of now, I've tested on the scenes we're expected to render and it looks like it works.
I was also able to render recursive_spheres_10, which I wouldn't recommend trying. I'm not super pleased with
my design since the handout explicitly says that having all of my gl__ calls in realtime.cpp is most likely bad,
but I'm not sure how I'd abstract it given the current structure (storing m_metaData and all the VBO/VAOs in the
realtime object). I didn't implement any extra credit and I'm not super disappointed by that, hopefully I'll feel
more inclined for project 6. One major change I made to my camera object from projects 3/4 to here is that I have
a separate setter and getting for the view, inverse view, and projection matrices, which (hopefully) prevents 
excess computation. I'm also certain that there's a way to cut back on the number of uniforms, maybe using more
vectors or structs, but it isn't too much of burden as of now. Maybe I'll revise that statement when I start
project 6 and the final project.
