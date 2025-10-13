

# information:
  Not this is work in progress build. Save and load data from event does not seem to work due how c lua works. It might be misconfig.
```
```

# Guide:



# Does work section here:
```c
{"set_data", l_enet_peer_set_data},   // Add set_data
{"get_data", l_enet_peer_get_data},   // Add get_data
```
  Set and get data is not working might be lua config c build.

- https://github.com/zpl-c/enet/blob/8647b6eaea881c86471ae29f732620d299fc20d7/include/enet.h#L687
