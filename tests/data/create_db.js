db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "00080018" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462499.1.0" ] },
      "0020000d" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.19057.1364461809.9999" ] },
      "0020000e" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462499.1" ] },
      "Content" : "" }
db.datasets.insert(j)

j = { "00080018" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462488.1.0" ] },
      "0020000d" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.19057.1364461809.8888" ] },
      "0020000e" : { "vr" : "UI", "Value" : [ "2.16.756.5.5.100.3611280983.20092.1364462488.1" ] },
      "Content" : "not_find" }
db.datasets.insert(j)

j = { "ae_title" : "LOCAL", "host" : "localhost", "port" : ${DOPAMINE_TEST_WRITINGPORT}}
db.application_entities.insert(j)

j = { "ae_title" : "REMOTE", "host" : "localhost", "port" : ${DOPAMINE_TEST_LISTENINGPORT}}
db.application_entities.insert(j)
