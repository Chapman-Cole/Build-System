# Build-System
A simple build system to make building projects across multiple machines easier for projects with long command line arguments.
In order to set it up, simply clone the repository, and compile the single c file on your machine using the compiler of your choice. The program only uses functions from the standard c libraries, specifically string.h, stdio.h, stdlib.h, and stdbool.h. This means it should work on most operating systems.
'

**An example for how the compilation would look is: gcc buildsys.c -o build.**


Once buildsys.c has been compiled to an executable, create a file with the exact name **build.set**. It is important that the file is spelled exactly like that and has no other hidden file extension. This file should be placed in the root directory (folder) of your project, and you should make sure that when you call
the executable you compiled earlier that **build.set** is in the same directory as the current directory you are in the terminal when you call the executable. 


For an example of what the **build.set** file syntax looks like, see below:

```
profile : profile_name = {
    src = main.c, glad.c, file2.c
    include = "C:\ProgrammingUtils\OpenGL\includes"
    libFolder = "C:\ProgrammingUtils\OpenGL\lib"
    libs = libglfw3.a, libgdi32.a
    out = main  
    compiler = gcc
    args = -Wall
}

profile : profile_name_two = {
    src = main.c, glad.c, file2.c
    out = main  
    compiler = gcc
}
```

A single build.set file can have multiple profiles defined by you so that if you have a project that you work on across different machines, then you can simply pass the name of the profile to the buildsys executable, and then it will specifically run the commands defined in that particular profile.


**For example, using the build.set shown above, you could execute the following build command: ./build profile_name_two.**


Another important thing to notice is that there are only 3 required settings per profile: src=, out=, compiler=. These are strictly necessary for the program to actually build an executable, and the first profile in the example above illustrates all available settings for a profile. 
