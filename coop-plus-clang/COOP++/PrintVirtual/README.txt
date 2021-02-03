This plugin shows information related to virtual call sites, virtual functions, and overriding information.

Build the plugin by running `make` in this directory, or use CMake.

Once the plugin is built, you can run it using:
--
Linux:
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.so -plugin print-virtual some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.so -plugin print-virtual -plugin-arg-print-virtual help -plugin-arg-print-virtual print-virtual-callsite some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.so -plugin print-virtual -plugin-arg-print-virtual print-virtual-fn -plugin-arg-print-virtual print-vf-member some-input-file.c

Mac:
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.dylib -plugin print-virtual some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.dylib -plugin print-virtual -plugin-arg-print-virtual help -plugin-arg-print-virtual print-virtual-callsite some-input-file.c
$ clang -cc1 -load ../../Debug+Asserts/lib/libPrintVirtual.dylib -plugin print-virtual -plugin-arg-print-virtual print-virtual-fn -plugin-arg-print-virtual print-vf-member some-input-file.c
