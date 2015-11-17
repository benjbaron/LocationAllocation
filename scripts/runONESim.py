#!/usr/bin/python
# -*- coding: utf8 -*-

# Script to start ONE simulations

from multiprocessing.dummy import Pool
from subprocess import Popen, PIPE, STDOUT
import optparse
import re

NB_PROCESSES = 8
regex = "(?:[\d\.ENaN\-]+\,)+[\d\.ENaN\-]+"

def run(settings, key, values):
    processes = []
    for v in values:
        settings[key] = v
        f = build_settings_file(settings)
        print f
        processes.append((v,Popen("echo \"{str}\" | java -jar ONE.jar -b {b} /dev/stdin".format(str=f,b=settings["b"]), shell=True,
                         stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)))

    def get_lines((v,process)):
        print "Execute for %s = %s" % (key, v)
        return process.communicate()[0] # get the output, [1] is the error

    outputs = Pool(NB_PROCESSES).map(get_lines, processes)
    # process the output of the simulations
    res = []
    for out in outputs:
        m = re.findall(regex, out)
        res+= m

    return res

def get_options():
    optParser = optparse.OptionParser()
    optParser.add_option("-f", "--file",  action="store", dest='file', help="setting file")
    optParser.add_option("-m", "--mobility",  action="store", dest='mobility', help='type of mobility')
    optParser.add_option("-d", "--deadline", action="store", dest='deadline', help='deadline of the requests', default=120, type="int")
    optParser.add_option("-s", "--max-storage", action="store", dest='maxStorage', help='maximum storage of the storage facilities', default=-1, type="int")
    optParser.add_option("-n", "--nb-facilities", action="store", dest='nrofFacilities', help='number of storage facilities', default=-1, type="int")
    optParser.add_option("-p", "--placement", action="store", dest='facilityPlacement', help='placement of the storage facilities', default="random")
    optParser.add_option("-r", "--rep", action="store", dest='replicationType', help='type of file replication', default="rep")
    optParser.add_option("-u", "--users", action="store", dest='nrofMobileUsers', help='number of mobile users', default=100)
    options, args = optParser.parse_args()
    return options

def build_settings_file(settings):
    # get the arguments and construct the variables
    if(settings["file"]):
        with open (settings["file"], "r") as f:
            return f.read()

    nrofHostGroups = 1
    movementModel = ""
    mobility = settings["mobility"]
    simTime = settings["simTime"]
    nrofMobileUsers = settings["nrofMobileUsers"]

    maxStorage = ""
    if(settings["maxStorage"] not in [-1, ""]):
        maxStorage = "FileSystemReport2.maxStorage = %s" % (settings["maxStorage"])

    if(settings["deadline"] != -1):
        deadline = settings["deadline"]
    else:
        deadline = "240"

    if(mobility == "WorkingDayMovement"):
        nrofHostGroups = 3
        # Define another group of nodes
        movementModel = """
MovementModel.worldSize = 5000, 5000

Group2.groupID = A
Group2.nrofHosts = 100
Group2.busControlSystemNr = 1
Group2.shoppingControlSystemNr = 1
Group2.nrOfOffices = 1
Group2.nrOfMeetingSpots = 1
Group2.movementModel = {mobility}
Group2.officeLocationsFile = filesystem/WDM-g1-offices.wkt
Group2.homeLocationsFile   = filesystem/WDM-g1-homes.wkt
Group2.meetingSpotsFile    = filesystem/WDM-g1-offices.wkt

Group3.groupID = B
Group3.nrofHosts = 25
Group3.busControlSystemNr = 2
Group3.shoppingControlSystemNr = 2
Group3.nrOfOffices = 3
Group3.nrOfMeetingSpots = 0
Group3.movementModel = {mobility}
Group3.officeLocationsFile = filesystem/WDM-g2-offices.wkt
Group3.homeLocationsFile   = filesystem/WDM-g2-homes.wkt
Group3.meetingSpotsFile    = filesystem/WDM-g2-offices.wkt
""".format(mobility=mobility)

    elif(mobility == "ManhattanGridMovement"):
        nrofHostGroups = 2
        movementModel = """
MovementModel.worldSize = 5000, 5000
Group2.groupID = t
Group2.nrofHosts = {nrofMobileUsers}
Group2.movementModel = {mobility}
ManhattanGridMovement.nbLines = 5
ManhattanGridMovement.nbColumns = 3
""".format(mobility=mobility, nrofMobileUsers=nrofMobileUsers)

    elif(mobility == "RandomWaypoint"):
        nrofHostGroups = 2
        movementModel = """
MovementModel.worldSize = 5000, 5000
Group2.groupID = t
Group2.nrofHosts = {nrofMobileUsers}
Group2.movementModel = RandomWaypoint
""".format(mobility=mobility, nrofMobileUsers=nrofMobileUsers)

    if(settings["facilityPlacement"] in ["random", "grid"]):
        facilityPlacement = settings["facilityPlacement"]
    if(settings["facilityPlacement"] == "grid" and mobility == "ManhattanGridMovement"):
        facilityPlacement = "manhattanGrid"

    if(settings["nrofFacilities"] != -1):
        nrofFacilities = settings["nrofFacilities"]
    else:
        nrofFacilities = "[1; 2; 3]" # "; 4; 5; 10; 15; 20; 25; 40; 45; 50; 75; 100; 125; 150]"

    str = """
Scenario.name = filesystem_scenario
Scenario.simulateConnections = true
Scenario.updateInterval = 0.1
Scenario.endTime = {simTime}

wifi.type = SimpleBroadcastInterface
wifi.transmitSpeed = 1M
wifi.transmitRange = 250

Events.nrof = 0
Scenario.nrofHostGroups = {nrofHostGroups}

Group.router = DirectDeliveryRouter
Group.nrofInterfaces = 1
Group.interface1 = wifi
Group.speed = 2.7, 13.9

Group1.groupID = f
Group1.nrofHosts = {nrofFacilities}
Group1.movementModel = StationaryMovement
# Group1.movementModel = RandomWaypoint
Group1.nodeLocation = -1, -1
{movementModel}
Report.nrofReports = 1
Report.warmup = 0
Report.reportDir = reports/

Report.report1 = FileSystemReport2
FileSystemReport2.reqGenRate = 0.016667
FileSystemReport2.deadline = {deadline}
FileSystemReport2.facilityPlacement = {facilityPlacement}
FileSystemReport2.allowNodeContacts = false
FileSystemReport2.useBackend = false
FileSystemReport2.replicationType = rep
FileSystemReport2.putPolicy = first
{maxStorage}
""".format(simTime=settings["simTime"], nrofHostGroups=nrofHostGroups, nrofFacilities=nrofFacilities, movementModel=movementModel, deadline=deadline, facilityPlacement=facilityPlacement, maxStorage=maxStorage)
    return str

if __name__ == '__main__':
    options = get_options()
    settings = {
        "file": options.file,
        "simTime": 30000,
        "deadline": "[60; 120; 200; 240; 600; 1200; 2400; 3600; 5000]",# options.deadline,
        "mobility": "RandomWaypoint", # "ManhattanGridMovement", # "RandomWaypoint",# options.mobility,
        "nrofFacilities": 50, # options.nrofFacilities,
        "nrofMobileUsers": 100,
        "b": 9,
        "maxStorage": -1,
        "facilityPlacement": "grid",
    }
    out = run(settings, "nrofFacilities", [1,2,3,4,5,10,25,35,50,75,100])
    for o in out:
        print o
