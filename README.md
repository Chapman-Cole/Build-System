# Build-System
A simple build system to make building projects across multiple machines easier for projects with long command line arguments.
In order to set it up, simply clone the repository, and compile the single c file on your machine using the compiler of your choice. The program only uses functions from the standard c libraries, specifically string.h, stdio.h, stdlib.h, and stdbool.h. This means it should work on most operating systems.
'

**An example for how the compilation would look is: gcc buildsys.c -o build.**


Once buildsys.c has been compiled to an executable, create a file with the exact name **build.set**. It is important that the file is spelled exactly like that and has no other hidden file extension. This file should be placed in the root directory (folder) of your project, and you should make sure that when you call
the executable you compiled earlier that **build.set** is in the same directory as the current directory you are in the terminal when you call the executable. 


For an example of what the **build.set** file syntax looks like, see below:

```
profile : gcc = {
    src = [main.c, glad.c, file2.c]
    include = ["C:\ProgrammingUtils\OpenGL\includes"]
    libFolder = ["C:\ProgrammingUtils\OpenGL\lib"]
    libs = [libglfw3.a, libgdi32.a]
    out = [main]  
    compilerPath = [gcc]
    args = [-Wall]
    compilerFormat = [gcc/clang]
    linkerOptions = [-Map, output.map]
}

profile : msvc = {
    src = [main.c, glad.c, file2.c]
    include = ["C:\ProgrammingUtils\OpenGL\includes"]
    libFolder = ["C:\ProgrammingUtils\OpenGL\lib"]
    libs = [glfw3.lib, kernel32.lib]
    out = [main]  
    compilerPath = [cl]
    compilerFormat = [msvc]
}

profile : test = {
    src = [
        main.c, 
        glad.c, 
        file2.c
    ]
    include = ["C:\Users\user1\C Programs"]
    out = [main]
    compilerPath = [gcc]
    compilerFormat = [gcc/clang]
}
```

A single build.set file can have multiple profiles defined by you so that if you have a project that you work on across different machines, then you can simply pass the name of the profile to the buildsys executable, and then it will specifically run the commands defined in that particular profile.


**For example, using the build.set shown above, you could execute the following build command: ./build profile_name_two.**


**Build Commands:**

    ./build <-- Runs the command associated with the first profile in build.set

    ./build profile_name <-- Runs the command associated with the specified profile in build.set

    ./build -printOnly <-- Will only print the generated command to the console that is associated with the first profile in build.set, but will not invoke the compiler

    ./build -printOnly profile_name <-- Will only print the generated command to the console that is associated with the specified profile in build.set, but will not invoke the compiler

    ./build -help <-- Will print a guide/overview of the tool to the console

Another important thing to notice is that there are only four required settings per profile: src=, out=, compilerPath=, and compilerFormat=. These are strictly necessary for the program to actually build an executable, and the first profile in the example above illustrates all available settings for a profile. 

**For Quick Reference, the help message printed by ./build -help is:**

------------------------------ Build System Settings and Instructions ------------------------------

First, in the root/parent directory (folder) of your project, create a file named build.set

It is very important it is spelled exactly like that and has no hidden file extensions.

In the build.set file that you created you can create a profile by typing 'profile : your_profile_name = {}'

In the build.set file you can have as many profiles as you would like, and just have to repeat
what was done above to create a new profile.

Inside the curly braces of the profile you have the following options:

src = [file1.c, file2.c, file3.c, etc.] <- necessary, the source files of your project

include = ["C:\ProgrammingUtils\OpenGL\includes", etc.] <- not required, the include folders for header files
in a different directory than the root directory of your project

libFolder = ["C:\ProgrammingUtils\OpenGL\lib"] <- not required, the directories of any libraries you wish to link to

libs = [libglfw3.a, libgdi32.a, etc.] <- not required, the names of the libraries you wish to link to (file extensions must be included)

out = [main] <- necessary, inside the brackets should be one item, which is the name of the output executable

compilerPath = [gcc] <- necessary, the path to the compiler of your choice

args = [-Wall] <- not required, any command line options you would like to pass to the compiler

compilerFormat = [gcc/clang] <- necessary, options for command formatting are 'gcc/clang' or 'msvc'

linkerOptions = [-Map, output.map] <- not required, any options you would like to pass to the linker

A basic build.set file would look like:
```
profile : your_profile_name = {
    src = [
        main.c,    
        file2.c,     
        file3.c       
    ]  
    compilerFormat = [gcc/clang]
    compilerPath = [gcc]
    out = [main]  
}
```
Note how elements inside the brackets can span multiple lines, as seen above with src =

Also note that by default spaces are removed unless you place the text inside quotes, like "C:\Users\My Programs"

When invoking this executable in the terminal you have the following command line options:

(None) -> if no command line arguments are passed into this executable, it will simply search for the first
          profile, generate its build command, and then run it.

profile -> you can pass in the name of one of the profile's you created, and it will search for that
           profile to generate the build command.

-printOnly -> This will generate the build command for the first profile in build.set and print it to the
              terminal, but it will not actually invoke the compiler (it is basically a preview).

-printOnly profile -> This will generate the build command for the specified profile, but will only
                      print it to the console and will not invoke the compiler.

-help -> prints out this message you are reading right now
