
1. Create or copy file heishamon.yaml in directory /config/packages ( if directory not exist , create it)
2. Open file configuration.yaml in directory /config ,and check for lines:

```
homeassistant:
  packages: !include_dir_named packages
```
If not exists ,add them.

3. Go to Configuration / Server Control ,and first check configuration , if it is ok Restart HA.
