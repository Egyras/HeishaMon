# How to add MQTT Topics for HeishaMon

1. Add a new topic line in file MQTT-Topics.md and define a new line on the table *Sensors Topics:*


ID | Topic | Response
--- | --- | ---
TOPxx | sdc/Unique_Topic_Name | description and response

2. Open README.md 
- find the Byte# and the decrypt rule in table *Protocol byte decrypt info:* according to the new topic
- add the new TOPxx in front of the line.

3. Open decode.h and add the new topic in. Make sure last in array does not end with comma (,) but all others do.

```
static const String topics[] = {"TOP0", //TOP0
                                .
                                .
                                "Unique_Topic_Name" //TOPxx)
                                };
```

```
static const unsigned int topicBytes[] = {0, //TOP0
                                            .
                                            .
                                          Byte# //TOPx
                                         };
```

```
static const topicFP topicFunctions[] = {unknown, //TOP0
                                            .
                                            .
                                          getRuleXXX //TOPxx
                                         };
```

4. Open decode.cpp and ad the #-part of TOPxx at the end of 

```
int topicsrun[] = {3, 4, 18, 17, ##};
```