import sys
import datetime
import os
from CppHeaderParser import CppHeader, CppParseError, CppVariable

fileStart = '''/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

// WARNING, this is an automatically generated file!
// Don't change anything in here.
// Last update {date}

# include <iostream>
# include <string>
'''
sensorSpecific = '''
# include \"{header}\"
// Configuration implementation for class {qfn}

{qfn}::{name}(const JsonObject &obj):
 {initializer}
  {{}}

void {qfn}::fillData(JsonObject &doc) const {{
  {filler}
}}

int {qfn}::getExpectedCapacity() const {{
  return JSON_OBJECT_SIZE({len});
}}
'''


def generateCodeForProperty(p: CppVariable, initializer: list, toString: list):
    if p['constant'] == 1:
        print("You can't have a const in the Configuration class!")
        exit(1)
    map = {'name': p["name"], 'default': p["default"]}
    if "default" in p:
        print("   Code for %s=%s" % (p["name"], p["default"]))
        initializer.append(
            "      {name}(obj[\"{name}\"] | {default})".format_map(map))
    else:
        print("   Code for %s" % p["name"])
        initializer.append(
            "      {name}(obj[\"{name}\"])".format_map(map))
    toString.append("  doc[\"{name}\"] = {name};".format_map(map))


def parseHeader(headerFullPath: str):
    header = os.path.basename(headerFullPath)
    directory = os.path.dirname(headerFullPath)+"/"
    try:
        cppHeader = CppHeader(directory+header)
    except CppParseError as e:
        print("Ignored {} because it failed to parse: {}".format(header, e))
        return
    sensorSpecifics = []
    for name, clazz in cppHeader.classes.items():
        initializer = []
        toString = []
        nameSpacePrefix = ""
        if clazz["namespace"]:
            nameSpacePrefix = clazz["namespace"]+"::"
        map = {'name': name, 'qfn': nameSpacePrefix+name, 'header': header}
        name = nameSpacePrefix+name
        print(name)
        foundConfigClass = False
        for inherit in clazz["inherits"]:
            if (nameSpacePrefix+inherit["class"] == "SHI::Configuration"):
                foundConfigClass = True
        if not foundConfigClass:
            print(
                "Ignoring {qfn} in file {header} as it is not a subclass of SHI::Configuration".format_map(map))
            continue
        for access, prop in clazz["properties"].items():
            print(" %s" % access)
            if (access == "public"):
                for p in prop:
                    print("  %s" % p)
                    generateCodeForProperty(p, initializer, toString)
            else:
                print("  Skipping non public members")
        map['len'] = len(initializer)
        map['initializer'] = ",\n".join(initializer)
        map['filler'] = "\n".join(toString)
        sensorSpecifics.append(sensorSpecific.format_map(map))

    if len(sensorSpecifics) > 0:
        outputFile = directory.replace(
            "/include/", "/src/")+header.replace(".h", "_config.cpp")
        print("Writing output to {}".format(outputFile))
        map['date'] = "{}".format(datetime.date.today())
        cpp = open(outputFile, 'w')
        print(fileStart.format_map(map), file=cpp)
        print("\n".join(sensorSpecifics), file=cpp)
        cpp.close()


basePath = "/Users/karstenbecker/PlatformIO/Projects/"
parseHeader(basePath+"SHIT/include/SHISensor.h")
parseHeader(basePath+"SmartHomeIntegration/include/SHIESP32HW.h")
parseHeader(basePath+"SHIMulticast/include/SHIMulticastHandler.h")
parseHeader(basePath+"SHIMQTT/include/SHIMQTT.h")
parseHeader(basePath+"SHIOpenhabRest/include/SHIOpenhabRestCommunicator.h")
parseHeader(basePath+"SHIBME680/include/SHIBME680.h")
