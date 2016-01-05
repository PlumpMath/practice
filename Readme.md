# Practice Programming

Every day I will do a small 30ish minute programming practice session. These are meant to be thrown away when I am done
and just to work on skills that help me when programming in the large. Use of libraries or prebuilt scaffolding are
permitted so I can focus on specific problems rather than boilerplate.

## Skills I wish to get better at.
  * System level programming in c.
  * Multithreaded programming
  * Graphics programming.
  * Algorithms and data structures.
  * Implementing things from papers.
  * Simulation code, physics stuff.
  * Data viz.
  * Maybe some audo stuff.

## Practice Log
* #####2015-01-04: Threaded Work Queue.
    
    **Problem**: Build a multithreaded circular work queue using processor atomics.

    **Time**: 37 minutes
    
    I managed to get a little linear single producer multiple consumer work queue up and
    running in half an hour. I didn't get to making it a ring buffer. It could be cool to
    try and build on this to implement different concurrency patterns but I don't know if
    that is against the rules or not. I feel like 30 minutes isn't much time and it takes
    about 30 mins to type out all the boilerplate to do this. We'll see though, maybe it's
    just because I'm rusty. Felt overall like a good first day.

* #####2015-01-05: OpenGL Cube.
    ![Image of 2015-01-05](https://dl.dropbox.com/s/hrn58urgk1c4y9k/Screenshot%202016-01-05%2016.43.47.png)

    **Problem**: Get a cube up on the screen with opengl.

    **Time**:     1 hour 21 minutes

    This took me a lot longer than I expected. Getting the shader code up and running was
    pretty easy, that was mostly a copy paste job of the code to handle errors. Then I had
    the cube vertices and the shader code ready to go but what took the longest is the math
    for the perspective transform. I tried for awhile to do it from scratch but ended up just
    copying in some code I already had to do it which also used a bunch of custom types. I
    would like to do this again with either a good math library that I decide I can use for
    practices like this or do it really from scratch and actually understand the math involved.
    I also ran into some problems with my lib segfaulting. I solved those by trying to link it
    int directly to the platform to get some errors saying functions were not defined. This is
    because those were copy paste functions that were decalred as inline because it was copied
    from a cpp project. Another error I ran into and couldn't figure out for awhile is that I wasn't
    calling glUseProgram on my shader program so nothing was happening. Opengl gives you no feedback
    and that took forever to figure out because afaik there's no easy way to debug opengl. Opengl
    kinda really sucks because of the no debugging thing, hopefully more practice will make it
    a lot less tedious though.
    

    