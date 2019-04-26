# Known Issues

## Parser

### Grammar

The grammar contains 13 shift/reduce conflicts.
Also, since the unit test matches any grammar rule, there are multiple reduce/reduce-conflicts. These will however not arise, when the testing mode isn't specified, because the parser can only match the unit-test grammar rule with the use of special "~" characters.


