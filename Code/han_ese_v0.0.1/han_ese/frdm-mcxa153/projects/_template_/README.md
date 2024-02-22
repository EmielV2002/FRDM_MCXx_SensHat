# Template project

This file provides a description how to create a new project based on this template project. Make sure to place the new project in the same folder as this template folder, because of the relative paths to files and folders in the projects.

1. Copy the content of this folder.
2. In the descriptions below, ```_template_``` must be renamed by the new  project name. The easiest way to do this is by search-replace.

## ARM GCC

a. In the file *CMakeLists.txt*
    - ```project(_template_)```
    - ```set(MCUX_SDK_PROJECT_NAME _template_.elf)```
    - Add source files, similar to main.c
    
        ```
        add_executable
        ```

## MCUXpresso

a. In the file *.project*
    - ```<name>_template_</name>```
    - Add linked resources, similar to main.c
    
        ```xml
        <link>
            <name>source/main.c</name>
            <type>1</type>
            <locationURI>PARENT-1-PROJECT_LOC/main.c</locationURI>
        </link>
        ```
## MDK

a. Rename *\_template_.uvoptx*
b. Rename ```_template_.uvprojx```
c. In the file ```_template_.uvprojx```
    - ```<TargetName>_template_ debug</TargetName>```
    - ```<OutputName>_template_.out</OutputName>- ```
    - ```<TargetName>_template_ release</TargetName>- ```
    - ```<OutputName>_template_.out</OutputName>- ```
    - Add source files, similar to main.c

        ```xml
        <File>
            <FileName>main.c</FileName>
            <FileType>1</FileType>
            <FilePath>../main.c</FilePath>
        </File>
        ```
