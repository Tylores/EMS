/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF), AllJoyn Open Source
 *    Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
******************************************************************************/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <map>
#include <chrono>
#include <thread>

#include <alljoyn/Status.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Observer.h>
#include <alljoyn/Init.h>

#include "utility.h"

#define INTF_NAME "edu.pdx.powerlab.demo"

using namespace std;
using namespace ajn;
using namespace qcc;

Utility tools;

/* convenience class that hides all the marshalling boilerplate from sight */
class AssetProxy {
    ProxyBusObject proxy;
    BusAttachment& bus;
  private:
    /* Private assigment operator - does nothing */
    AssetProxy operator=(const AssetProxy&);
  public:
    map<string,double> properties;

    AssetProxy(ProxyBusObject proxy, BusAttachment& bus) : proxy(proxy), bus(bus){
        proxy.EnablePropertyCaching();
    }

    bool IsValid() {
        return proxy.IsValid();
    }

    QStatus ImportPower(double t_watts) {
        cout << "Sending desired export of: " << t_watts << endl;
        Message reply(bus);
        MsgArg arg("d", t_watts);
        QStatus status = proxy.MethodCall(INTF_NAME, "ImportPower", &arg, 1, 0);
        return status;
    }

    QStatus ExportPower(double t_watts) {
        cout << "Sending desired export of: " << t_watts << endl;
        Message reply(bus);
        MsgArg arg("d", t_watts);
        QStatus status = proxy.MethodCall(INTF_NAME, "ExportPower", &arg, 1, 0);
        return status;
    }

    QStatus SignalPJM(double tValue){
	Message reply(bus);
	MsgArg arg("d", tValue);
	QStatus status = proxy.MethodCall(INTF_NAME, "SignalPJM", &arg, 1, 0);
	return status;
    } // end signal pjm

    void pathDelim(){
        vector <string> path, temp;
        int num;
        double val;
        path = tools.stringDelim(proxy.GetPath().c_str(),'/');
        num = path.size();
        for (int i = 0; i < num; i++){
            if (path[i].find("region") == 0){
                temp = tools.stringDelim(path[i],'_');
                val = stod(temp[1]);
                properties.insert(pair<string,double>("region",val));
            } else if (path[i].find("feeder") == 0){
                temp = tools.stringDelim(path[i],'_');
                val = stod(temp[1]);
                properties.insert(pair<string,double>("feeder",val));
            }
        } // end string comparison
    } // END PATH DELIM

    void telemetry() {
        properties.clear();
        //cout << "Polling asset telemetry " << endl;
        MsgArg dict;

        QStatus status = proxy.GetAllProperties(INTF_NAME, dict);
        if (ER_OK == status){
            pathDelim();
            MsgArg * entries = NULL;
            size_t num = 0;
            dict.Get("a{sv}", &num, &entries);
            for (size_t i = 0; i < num; ++i) {
                char * key;
                double val;
                status = entries[i].Get("{sd}", &key, &val);
                if (ER_OK == status) {
                    properties.insert(pair<string,double>(key,val));
                }
            }
        }
    } // END TELEMETRY

    vector<double> GetProperties() {
	vector<double> values;
	values.reserve(properties.size());
        for(auto it = properties.cbegin(); it != properties.cend(); ++it)
        {
            values.emplace_back(it->second);
        }

	return values;
    } // END PRINT TELEMETRY
};

static void Help()
{
    cout << "q                  quit" << endl;
    cout << "l                  list all discovered Assets" << endl;
    cout << "t                  print telemetry" << endl;
    cout << "i <int watts>      import power signal" << endl;
    cout << "e <int watts>      export power signal" << endl;
    cout << "x <int watts>      import power test" << endl;
    cout << "y <int watts>      export power test" << endl;
    cout << "h                  display this help message" << endl;
}

unsigned int ListAssets(BusAttachment& bus, Observer* observer)
{
    unsigned int ctr = 0;
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy)) {
        ctr++;
        AssetProxy asset(proxy, bus);
    } // for

    cout << ctr << endl;
    return  ctr;
}

vector<vector<double>> Telemetry(BusAttachment& bus, Observer* observer)
{
    unsigned int count = ListAssets(bus, observer);
    vector<vector<double>> assets;
    assets.reserve(count);

    vector<double> data;
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy))
    {
        AssetProxy asset(proxy, bus);
        asset.telemetry();
        data = asset.GetProperties();
	assets.emplace_back(data);
    } //for

    string file = "save_";
    file.append(tools.GetTime());
    tools.StoreData(assets, file);
    return assets;
} // END TELEMETRY

static void CallImportPower(BusAttachment& bus, Observer* observer, double t_watts)
{
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy)) {
        AssetProxy asset(proxy, bus);
        QStatus status = asset.ImportPower(t_watts);
        if (ER_OK != status) {
            cerr << "Could not set desired import power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
            continue;
        }
    }
}

static void TestImport(BusAttachment& bus, Observer* observer, double t_watts)
{
    QStatus status;
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy)) {
        if (!strcmp(proxy.GetPath().c_str(),"/asset_bess/region_1/feeder_1")){
            AssetProxy asset(proxy, bus);
            asset.telemetry();
            while (asset.properties["import_energy"] >= 0){
                cout << "The import energy is: " << asset.properties["import_energy"] << endl;
                status = asset.ImportPower(t_watts);
                if (ER_OK != status) {
                    cerr << "Could not set desired import power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
                    continue;
                }
                // wait one hour and then tell the system to stop with a zero signal.
                this_thread::sleep_for(chrono::seconds(10));

                status = asset.ImportPower(0);
                if (ER_OK != status) {
                    cerr << "Could not set desired import power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
                    continue;
                }
                asset.telemetry();
            }
        }
    }
    cout <<"/n/n/n****TEST COMPLETE****/n/n/n";
}

static void TestExport(BusAttachment& bus, Observer* observer, double t_watts)
{
    QStatus status;
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy)) {
        if (!strcmp(proxy.GetPath().c_str(),"/asset_bess/region_1/feeder_1")){
            AssetProxy asset(proxy, bus);
            asset.telemetry();
            while (asset.properties["export_energy"] >= 0){
                cout << "The export energy is: " << asset.properties["export_energy"] << endl;
                status = asset.ExportPower(t_watts);
                if (ER_OK != status) {
                    cerr << "Could not set desired export power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
                    continue;
                }
                // wait one hour and then tell the system to stop with a zero signal.
                this_thread::sleep_for(chrono::seconds(10));

                status = asset.ExportPower(0);
                if (ER_OK != status) {
                    cerr << "Could not set desired export power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
                    continue;
                }
                asset.telemetry();
            }
        }
    }
    cout <<"/n/n/n****TEST COMPLETE****/n/n/n";
}

static void CallExportPower(BusAttachment& bus, Observer* observer, double t_watts)
{
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy)) {
        AssetProxy asset(proxy, bus);
        QStatus status = asset.ExportPower(t_watts);
        if (ER_OK != status) {
            cerr << "Could not set desired export power " << proxy.GetUniqueName() << ":" << proxy.GetPath() << endl;
            continue;
        }
    }
}

static void RegD(BusAttachment& bus, Observer* observer, double tValue)
{
    ProxyBusObject proxy = observer->GetFirst();
    for (; proxy.IsValid(); proxy = observer->GetNext(proxy))
    {
	AssetProxy asset(proxy, bus);
	QStatus status = asset.SignalPJM(tValue);
	if (ER_OK != status){
	    cerr << "Could not send RegD to: " << proxy.GetUniqueName() << endl;
	   continue;
	} // if

    } // for

} // end RegD

static void PJMControl(BusAttachment& bus, Observer* observer)
{
    //call collect data function
    vector<vector<double>> data = Telemetry(bus, observer);
    //call aggregator
    vector<double> totals = tools.Aggregate(data);

    string file = "samplePJM.txt";
    vector<double> schedule = tools.ImportSchedule(file);

    for(unsigned int i=0; i<schedule.size(); i++)
    {
    	auto startTime = chrono::high_resolution_clock::now();
	//send signal function
	RegD(bus, observer, schedule[i]);
	//emulate models
	auto endTime = chrono::high_resolution_clock::now();
    	chrono::duration<double, milli> elapsed = startTime - endTime;
	int wait = 2000 - elapsed.count();		// 2 seconds - processing time
   	this_thread::sleep_for(chrono::milliseconds(wait));
    } //for

    //call collect data function
    data = Telemetry(bus, observer);
} // END PJM CONTROL


static bool Parse(BusAttachment& bus, Observer* observer, const string & input)
{
    char cmd;
    vector <string> tokens;
    string option;

    if (input == "") {
        return true;
    }

    stringstream s(input);
    while(!s.eof()) {
        string tmp;
        s >> tmp;
        tokens.push_back(tmp);
    }

    if (tokens.empty()) {
        return true;
    }

    cmd = input[0];

    switch (cmd) {
    case 'q':
        return false;

    case 'l':
        ListAssets(bus, observer);
        break;

    case 't':
        Telemetry(bus, observer);
        break;

    case 'i':
        if (tokens.size() < 2) {
            Help();
            break;
        }
        option = tokens.at(1);
        CallImportPower(bus, observer,stod(option));
        break;

    case 'e':
        if (tokens.size() < 2) {
            Help();
            break;
        }
        option = tokens.at(1);
        CallExportPower(bus, observer,stod(option));
        break;

    case 'x':
        if (tokens.size() < 2) {
            Help();
            break;
        }
        option = tokens.at(1);
        TestImport(bus, observer,stod(option));
        break;

    case 'y':
        if (tokens.size() < 2) {
            Help();
            break;
        }
        option = tokens.at(1);
        TestExport(bus, observer,stod(option));
        break;

    case 's':
	PJMControl(bus, observer);
	break;

    case 'h':
    default:
        Help();
        break;
    }

    return true;
}

static QStatus BuildInterface(BusAttachment& bus)
{
    QStatus status;

    InterfaceDescription* intf = NULL;
    status = bus.CreateInterface(INTF_NAME, intf);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddMethod("ImportPower", "d", NULL, "watts", MEMBER_ANNOTATE_NO_REPLY);
    assert (ER_OK == status);

    status = intf->AddMethod("ExportPower", "d", NULL, "watts", MEMBER_ANNOTATE_NO_REPLY);
    assert (ER_OK == status);

    status = intf->AddMethod("SignalPJM", "d", NULL, "RegD", MEMBER_ANNOTATE_NO_REPLY);
    assert (ER_OK == status);

    status = intf->AddProperty("import_power", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("import_power", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("export_power", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("export_power", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("import_energy", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("import_energy", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("export_energy", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("export_energy", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("idle_characteristic", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("idle_characteristic", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("import_characteristic", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("import_characteristic", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("export_characteristic", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("export_characteristic", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    status = intf->AddProperty("time", "d", PROP_ACCESS_READ);
    QCC_ASSERT(ER_OK == status);

    status = intf->AddPropertyAnnotation("time", "org.freedesktop.DBus.Property.EmitsChangedSignal", "false");
    QCC_ASSERT(ER_OK == status);

    intf->Activate();

    return status;
}

static QStatus SetupBusAttachment(BusAttachment& bus)
{
    QStatus status;
    status = bus.Start();
    QCC_ASSERT(ER_OK == status);
    status = bus.Connect();

    if (ER_OK != status) {
        return status;
    }

    status = BuildInterface(bus);
    QCC_ASSERT(ER_OK == status);

    return status;
}

class AssetListener :
    public MessageReceiver,
    public Observer::Listener {
    static const char * props[];
  public:
    Observer* observer;
    BusAttachment* bus;

    virtual void ObjectDiscovered(ProxyBusObject& proxy) {
        cout << "[listener] Asset " << proxy.GetUniqueName() << ":"
             << proxy.GetPath() << " has just been discovered." << endl;
    }

    virtual void ObjectLost(ProxyBusObject& proxy) {
        cout << "[listener] Asset " << proxy.GetUniqueName() << ":"
             << proxy.GetPath() << " no longer exists." << endl;
    }
};

const char* AssetListener::props[] = {
    "IsOpen"
};

int CDECL_CALL main(int argc, char** argv)
{
    QCC_UNUSED(argc);
    QCC_UNUSED(argv);

    if (AllJoynInit() != ER_OK) {
        return EXIT_FAILURE;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return EXIT_FAILURE;
    }
#endif

    BusAttachment* bus = new BusAttachment("Asset_consumer", true);

    if (ER_OK != SetupBusAttachment(*bus)) {
        return EXIT_FAILURE;
    }

    const char* intfname = INTF_NAME;
    Observer* obs = new Observer(*bus, &intfname, 1);
    AssetListener* listener = new AssetListener();
    listener->observer = obs;
    listener->bus = bus;
    obs->RegisterListener(*listener);

    bool done = false;
    printf("\n\n\n**********\tEMS\t**********\n\n\n");
    Help();
    printf("\n\n\n");
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !Parse(*bus, obs, input);
    }

    // Cleanup
    obs->UnregisterAllListeners();
    delete obs; // Must happen before deleting the original bus
    delete listener;
    delete bus;
    bus = NULL;

#ifdef ROUTER
    AllJoynRouterShutdown();
#endif
    AllJoynShutdown();
    return EXIT_SUCCESS;

}
