CMPSC 473 - Project #1 - C Object Processing

Due Date: Th September 19, 2019 at 11:59pm.

In this project, you will build a program that uploads and retrieves objects by name requiring a variety of C memory operations.

Environment: We will be running your code on the W204 Westgate Linux lab machines so you must check that your code runs successfully on one of those machines. These machines are named cse-p204instXX.cse.psu.edu, where XX is a number between 01 and about 40. You can remotely login to any of these machines (need VPN connection if off campus), and all these machines should be identical.

But, you may develop your code using a Vagrant VM on your computer to avoid having to remotely login. Here's the link that can guide you to set up your environment.

Inputs: Download the tarball for Project 1 to obtain the input server code. The code consists of three files - only one of which you will need to modify.

    The file cmpsc473-main.c implements the interface code to uploads and retrieves data objects. You will have to complete functions in this file, as described below.

    The file cmpsc473-kvs.c implements a key-value store that you will use to store and retrieve server objects. The API includes kvs_auth_set, which binds a key to a value, which are stored as a unit in the KVS. The key is used to retrieve the value using the API kvs_auth_get, which retrieves a value given a key as input.

    The file cmpsc497-utils.c implements some helper functions. You will not necessarily have to use these yourself, but it is up to you.

In addition, you will be provided with your own data format for your data objects. Each student will receive their own object format - defined by the file cmpsc473-format-x.h, where x is a number identifying a different format. This file will be available on Tu Sept. 3. The object format consists of a base structure (struct A) and one or more fields that reference data of strings, ints, or other structures. You will also receive an input file consisting of one object with data values in that format.

Server API Your server program must implement two APIs from the command line:

    ./cmpsc473-p1 set [object-name] [object-file]

    The "set" command will add an object provided in the [object-file] to the "Objects" KVS (as described below) under the name [object-name].

    ./cmpsc473-p1 get [object-name]

    The "get" command will retrieve an object from the "Objects" KVS that matches the [object-name] provided. 

Project Tasks: The overall goal of this project is to build a program that enables users to upload and retrieve objects from a key-value store, where objects will adhere to your specified format.

In building the program, you will have to complete the following tasks:

    Edit the Makefile (TASK 0) for submitting the project. Your student number associated with your format file must be assigned to the variable SNUM in the Makefile.

    Implement the "upload_X" functions (TASK 1) invoked from the function set_object. Each upload function will read the file from the provided file pointer line-by-line to populate the fields of the respective data structure based on your unique format file cmpsc473-format-x.h (i.e., each student has their own format file). For example, upload_A allocates an instance of an object of the type struct A and reads each line in the provided input file to assign values to each field (i.e., for int and character string fields).

    Note that structures of type struct A have pointer fields to reference other objects and for function pointers. You will populate these fields (TASK 2) at the end of the function set_object, after all the objects have been created. Set the function pointers in struct A for op0 and op1 to enable invocation of the functions output_obj and marshall, respectively.

    Implement the function marshall (TASK 3) to prepare the object data for the persistent storage in the "Objects" KVS. marshall converts an object to a format suitable for storage or transmission. Specifically, marshall creates an linearizes the structure fields in struct A into a series of data items for supplying data values for storage in the "Objects" KVS that can be recreated in memory. The requirement is to copy the data from each field in objects of type struct A into a contiguous memory space. Copy the data from the data fields (int and char[]) of struct A and copy the structured data from structs B and C. This memory will be stored in the KVS and written to the file system each time the program terminates.

    Since you each have different object formats, you will have to determine the size of your linearized object and assign that to the #define OBJ_LEN at the top of cmpsc473-main.c.

    Implement the function unmarshall (TASK 4) to convert a linearized object from the "Objects" KVS back into its data structure form. This is the opposite process from marshall above.

    Implement the function output_obj (TASK 5) to print object information as the result of a "get" operation. Write a function to print all the integer fields (int) of struct A, plus all the fields (int and char) of structs B and C. A partial example of code for a different structure format is provided in the output_obj in the assigned file show you how I want it to be done. You need to follow this format, changing only the field names for us to test your code automatically. There will be a penalty if you do not output in the proper format.

Outputs:

Your program must enable the following functionality:

    A "set" operation adds a new object to the "Objects" KVS in set_object. The object must be formatted using marshall prior to adding it to the "Objects" KVS. Objects are stored in the KVS by their object name (no more than 8 characters). The marshalled data for all the object data rooted by the struct A object is stored as the value in the KVS (value).

    NOTE: If there are two "set" operations for the same identifier, the object stored will include the latest data. So, a "get" operation can only retrieve one object of a particular name.

    It must be possible to retrieve that object data with a subsequent "get" operation defined by get_object. A "get" operation must use unmarshall to create the in-memory data structures for the objects, and output all the int fields for struct A and all the int and char (string) fields for structs B and C using output_obj.

Issues to Check for: We are expecting that your code will process inputs correctly for strings and integers. Since the input file can have arbitrarily long strings and arbitrary integer values, you need to check these. We will be testing that your program will not crash due to such inputs.

    Strings: must write no more than STRLEN bytes into the string. Please truncate the input string. We will not be checking for specific error messages, but you are free to print an error message.

    Integers: must be a legal integer value for the platform between INT_MIN and INT_MAX. If not, then set the value to 0.
