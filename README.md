# Implementation-of-multithreading-functionality-in-a-data-storage-engine

The provided source code (https://github.com/nopper/kiwi/) implements the Kiwi storage engine, which is based on a log-structured merge (LSM) tree. Storage engines are a significant component of modern cloud infrastructures as they are responsible for storing and retrieving data on local devices of a machine.

We implement the multithreading functionality of the PUT and GET commands provided by the storage engine. Our implementation allows multiple threads to concurrently invoke the "put" and "get" commands. The storage engine should correctly execute the concurrent operations and maintain statistics regarding the execution time of each operation. The implementation should be done in the C language and utilize the Pthreads library in Linux.

Based on the tutor's detailed project description (see ProjectDescription.pdf file)
the implementation, on the wiki repository, is provided via the Report.pdf file, which explain step by step the changes in souce code.
