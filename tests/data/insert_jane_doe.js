db = connect("localhost:27017/${DOPAMINE_TEST_DATABASE}");
j = { "00080008" : { "vr" : "CS", "Value" : [  "ORIGINAL",  "PRIMARY",  "OTHER" ] },
      "00080012" : { "vr" : "DA", "Value" : [  "20140827" ] },
      "00080013" : { "vr" : "TM", "Value" : [  "103310" ] },
      "00080016" : { "vr" : "UI", "Value" : [  "1.2.840.10008.5.1.4.1.1.4" ] },
      "00080018" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1.0" ] },
      "00080020" : { "vr" : "DA", "Value" : [  "20130328" ] },
      "00080022" : { "vr" : "DA", "Value" : [  "20130328" ] },
      "00080030" : { "vr" : "TM", "Value" : [  "101009" ] },
      "00080032" : { "vr" : "TM", "Value" : [  "101818" ] },
      "00080050" : { "vr" : "SH", "Value" : null },
      "00080060" : { "vr" : "CS", "Value" : [  "MR" ] },
      "00080070" : { "vr" : "LO", "Value" : [  "Bruker BioSpin MRI GmbH" ] },
      "00080080" : { "vr" : "LO", "Value" : [  "STRASBOURG" ] },
      "00080090" : { "vr" : "PN", "Value" : [  {  "Alphabetic" : "Gregory^House" } ] },
      "00081010" : { "vr" : "SH", "Value" : [  "Station" ] },
      "00100010" : { "vr" : "PN", "Value" : [  {  "Alphabetic" : "Doe^Jane" } ] },
      "00100020" : { "vr" : "LO", "Value" : [  "dopamine_test_01" ] },
      "00100030" : { "vr" : "DA", "Value" : null },
      "00100040" : { "vr" : "CS", "Value" : [  "F" ] },
      "00101030" : { "vr" : "DS", "Value" : [  5 ] },
      "00180020" : { "vr" : "CS", "Value" : [  "RM",  "IR" ] },
      "00180021" : { "vr" : "CS", "Value" : [  "NONE" ] },
      "00180022" : { "vr" : "CS", "Value" : null },
      "00180023" : { "vr" : "CS", "Value" : [  "2D" ] },
      "00180024" : { "vr" : "SH", "Value" : [  "FAIR_EPI (pvm)" ] },
      "00180050" : { "vr" : "DS", "Value" : [  0.8 ] },
      "00180080" : { "vr" : "DS", "Value" : [  18000 ] },
      "00180081" : { "vr" : "DS", "Value" : [  33 ] },
      "00180082" : { "vr" : "DS", "Value" : [  35.37627273 ] },
      "00180083" : { "vr" : "DS", "Value" : [  1 ] },
      "00180084" : { "vr" : "DS", "Value" : [  200.3334861 ] },
      "00180085" : { "vr" : "SH", "Value" : [  "1H" ] },
      "00180088" : { "vr" : "DS", "Value" : [  0.8 ] },
      "00180089" : { "vr" : "IS", "Value" : [  107 ] },
      "00180091" : { "vr" : "IS", "Value" : [  107 ] },
      "00180094" : { "vr" : "DS", "Value" : [  100 ] },
      "00180095" : { "vr" : "DS", "Value" : [  3337.783712 ] },
      "00181020" : { "vr" : "LO", "Value" : [  "ParaVision 5.1" ] },
      "00181030" : { "vr" : "LO", "Value" : [  "Protocol" ] },
      "00181310" : { "vr" : "US", "Value" : [  107,  0,  0,  107 ] },
      "00181312" : { "vr" : "CS", "Value" : [  "COL" ] },
      "00181314" : { "vr" : "DS", "Value" : [  90 ] },
      "00185100" : { "vr" : "CS", "Value" : [  "HFS" ] },
      "0020000d" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.19057.1364461809.7789" ] },
      "0020000e" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1" ] },
      "00200010" : { "vr" : "SH", "Value" : [  "Study_id" ] },
      "00200011" : { "vr" : "IS", "Value" : [  196609 ] },
      "00200012" : { "vr" : "IS", "Value" : [  1 ] },
      "00200013" : { "vr" : "IS", "Value" : [  1 ] },
      "00200032" : { "vr" : "DS", "Value" : [  -15,  -15,  -1.6 ] },
      "00200037" : { "vr" : "DS", "Value" : [  1,  6.123031769e-17,  0,  -6.123031769e-17,  1,  0 ] },
      "00200052" : { "vr" : "UI", "Value" : [  "2.16.756.5.5.100.3611280983.20092.1364462458.1.6.15.18" ] },
      "00201002" : { "vr" : "IS", "Value" : [  75 ] },
      "00201040" : { "vr" : "LO", "Value" : null },
      "00201041" : { "vr" : "DS", "Value" : [  -1.6 ] },
      "00280002" : { "vr" : "US", "Value" : [  1 ] },
      "00280004" : { "vr" : "CS", "Value" : [  "MONOCHROME2" ] },
      "00280010" : { "vr" : "US", "Value" : [  128 ] },
      "00280011" : { "vr" : "US", "Value" : [  128 ] },
      "00280030" : { "vr" : "DS", "Value" : [  0.234375,  0.234375 ] },
      "00280100" : { "vr" : "US", "Value" : [  16 ] },
      "00280101" : { "vr" : "US", "Value" : [  16 ] },
      "00280102" : { "vr" : "US", "Value" : [  15 ] },
      "00280103" : { "vr" : "US", "Value" : [  1 ] },
      "00280106" : { "vr" : "US", "Value" : [  3 ] },
      "00280107" : { "vr" : "US", "Value" : [  32766 ] },
      "00281050" : { "vr" : "DS", "Value" : [  16385 ] },
      "00281051" : { "vr" : "DS", "Value" : [  32764 ] },
      "00281055" : { "vr" : "LO", "Value" : [  "MinMax" ] },
      "Content" : new BinData(0,"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") }
db.datasets.insert(j)
