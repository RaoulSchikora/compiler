# Known Issues

## Parser

The grammar works without memory leaks, with the following issues still unadressed:

### Return statement

Empty return statements are supported by the datastructure, but currently lead to segmentation faults

