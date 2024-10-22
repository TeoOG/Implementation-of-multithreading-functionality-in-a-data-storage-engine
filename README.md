# Implementation-of-multithreading-functionality-in-a-data-storage-engine

The provided source code (https://github.com/nopper/kiwi/) implements the Kiwi storage engine, which is based on the log-structured merge (LSM) tree. Storage engines are a significant component of modern cloud infrastructures, as they are responsible for storing and retrieving data on local devices of a machine.

We implement the multithreading functionality of the **PUT** and **GET** commands, provided by the storage engine. Our implementation allows multiple threads to concurrently invoke each one of those commands. We added a new functionality, **readwrite** that implements the above two commands at once. Safety measures were taken, like implementing a solution to the readers-writers problem (writer's preferred solution) to prevent race conditions and starvation and to ensure data consistency and security.  The storage engine correctly executes the said operations and maintains some statistics regarding the execution time of each operation. The implementation is done in the C language and utilizes the Pthread library on Linux.

This repository only provides the modified files, from the **bench** and **engine** components of the https://github.com/nopper/kiwi/ repository, and not the whole engine. In order to test, replace those modified files with the original files in the engine.
