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

#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sstream>
#include <ctime>
#include <thread>
#include <chrono>

#include <alljoyn/Status.h>
#include <alljoyn/AboutObj.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/Init.h>

#include "BESS.h"

using namespace std;
using namespace ajn;

/*constants*/
static const char * INTF_NAME = "edu.pdx.powerlab.demo";
static const char * SERVICE_PATH = "/asset_bess/region_1/feeder_1";
SessionPort SERVICE_PORT = 25;
bool done = false;


/******************************************************************************
 *      Session Port Listener
 *      ---------------------
 *      AllJoyn class to join a session with the EMS
******************************************************************************/
class SPL : public SessionPortListener {
    virtual bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts) {
        QCC_UNUSED(sessionPort);
        QCC_UNUSED(joiner);
        QCC_UNUSED(opts);
        return true;
    }
};

SPL g_session_port_listener;



/******************************************************************************
 *      Build Interface
 *      ---------------
 *      AllJoyn function to establish the Methods/Properties/Signals that will
 *      be implemented by the BusObject.
 *
 *      @Method(ImportPower): A control signal from the EMS with a desired watt
 *          value that needs to be consumed by the asset for an hour.
 *      @Method(ExportPower): A control signal fromt he EMS with a desired watt
 *          value that needs to be generated/shed by the asset for an hour.
 *      @Property(import_power): The assets rated input power capacity in watts.
 *      @Property(export_power): The assets rated generation/shed power capacity
 *          in watts.
 *      @Property(import_energy): The assets total available input power capacity
 *          in watt-hours.
 *      @Property(export_energy): The assets total available generation/shed power
 *          capacity in watt-hours.
 *      @Property(import_characteristic): The assets input power response approximated
 *          as a linear slope.
 *      @Property(export_characteristic): The assets generation/shed response
 *          approximated as a linear slope.
 *      @Property(time): A timestampe used to coordinate the aggregated assets.
******************************************************************************/
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



/******************************************************************************
 *      Setup Bus Attachment
 *      --------------------
 *      AllJoyn function to connect to a DBus session and populate the about
 *      properties of the asset.
******************************************************************************/
static QStatus SetupBusAttachment(BusAttachment& bus, AboutData& aboutData)
{
    QStatus status;
    status = bus.Start();
    QCC_ASSERT(ER_OK == status);
    status = bus.Connect();
    if (status != ER_OK) {
        return status;
    }

    status = BuildInterface(bus);
    QCC_ASSERT(ER_OK == status);

    SessionOpts opts(SessionOpts::TRAFFIC_MESSAGES, false, SessionOpts::PROXIMITY_ANY, TRANSPORT_ANY);
    bus.BindSessionPort(SERVICE_PORT, opts, g_session_port_listener);

    /* set up totally uninteresting about data */
    //AppId is a 128bit uuid
    uint8_t appId[] = { 0x01, 0xB3, 0xBA, 0x14,
                        0x1E, 0x82, 0x11, 0xE4,
                        0x86, 0x51, 0xD1, 0x56,
                        0x1D, 0x5D, 0x46, 0xB0 };
    aboutData.SetAppId(appId, 16);
    aboutData.SetDeviceName("Foobar 2000 Door Security");
    //DeviceId is a string encoded 128bit UUID
    aboutData.SetDeviceId("93c06771-c725-48c2-b1ff-6a2a59d445b8");
    aboutData.SetAppName("Application");
    aboutData.SetManufacturer("Manufacturer");
    aboutData.SetModelNumber("123456");
    aboutData.SetDescription("A poetic description of this application");
    aboutData.SetDateOfManufacture("2014-03-24");
    aboutData.SetSoftwareVersion("0.1.2");
    aboutData.SetHardwareVersion("0.0.1");
    aboutData.SetSupportUrl("http://www.example.org");
    if (!aboutData.IsValid()) {
        cerr << "Invalid about data." << endl;
        return ER_FAIL;
    }
    return status;
}



/******************************************************************************
 *      Asset (BusObject, Asset)
 *      ------------------------
 *      The asset is the BusObject as well as asset. Therefore it inherits
 *      each of the parent class functions.
 *
 *      BusObject(Get, ImportPowerHandler, ExportPowerHandler)
 *
 *      Asset(State, Mode, Properties, Logging)
 *          @State : Checks the health of the asset to determine its Mode.
 *              i.e.(bypass, backup, nominal, disconnected)
 *          @Mode : The asset Mode dictates what it can participate in.
 *          @Properties : The asset polls its subsystems to determine the BusObject
 *              properties.
 *          @Logging : The system stores error/data locally for system analysis
 *              and troubleshooting.
******************************************************************************/
class Asset : public BusObject, public BESS {
  private:
    BusAttachment& bus;

  public:
    Asset(BusAttachment& bus)
        : BusObject(SERVICE_PATH),
        bus(bus)
    {
        const InterfaceDescription* intf = bus.GetInterface(INTF_NAME);
        QCC_ASSERT(intf);
        AddInterface(*intf, ANNOUNCED);

        /** Register the method handlers with the object **/
        const MethodEntry methodEntries[] = {
            { intf->GetMember("ImportPower"), static_cast<MessageReceiver::MethodHandler>(&Asset::ImportPowerHandler) },
            { intf->GetMember("ExportPower"), static_cast<MessageReceiver::MethodHandler>(&Asset::ExportPowerHandler) },
        };
        QStatus status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(methodEntries[0]));
        if (ER_OK != status) {
            cerr << "Failed to register method handlers for Asset." << endl;
        }
    }

    ~Asset()
    {
    }

    /* property getters */
    QStatus Get(const char*ifcName, const char*propName, MsgArg& val)
    {
        if (strcmp(ifcName, INTF_NAME)) {
            return ER_FAIL;
        }

        if (!strcmp(propName, "import_power")) {
            val.Set("d", m_importWatts);
        } else if (!strcmp(propName, "export_power")) {
            val.Set("d", m_exportWatts);
        } else if (!strcmp(propName, "import_energy")) {
            val.Set("d", m_importWattHours);
        } else if (!strcmp(propName, "export_energy")) {
            val.Set("d", m_exportWattHours);
        } else if (!strcmp(propName, "idle_characteristic")) {
            val.Set("d", m_idleChar);
        } else if (!strcmp(propName, "import_characteristic")) {
            val.Set("d", m_importChar);
        } else if (!strcmp(propName, "export_characteristic")) {
            val.Set("d", m_exportChar);
        } else if (!strcmp(propName, "time")) {
            double t = static_cast<unsigned long int> (time(NULL));
            val.Set("d", t);
        } else {
            return ER_FAIL;
        }
        return ER_OK;
    }

    void ImportPowerHandler(const InterfaceDescription::Member * t_member, Message & t_msg)
    {
        QCC_UNUSED(t_member);
        m_exportControl = 0;
        m_importControl = t_msg->GetArg(0)->v_double;
        printf("\t***Import Power Method recieved with a desired setting of %g.\n\n\n", m_importControl);
    } //END IMPORT POWER HANDLER

    void ExportPowerHandler(const InterfaceDescription::Member * t_member, Message & t_msg)
    {
        QCC_UNUSED(t_member);
        m_importControl = 0;
        m_exportControl = t_msg->GetArg(0)->v_double;
        printf("\t***Export Power Method recieved with a desired setting of %g.\n\n\n", m_exportControl);
    } //END EXPORT POWER HANDLER

    void AssetControl(){
        Loop();
    } //END ASSET CONTROL
};



/******************************************************************************
 *      Help
 *      ----
 *      Display input criteria for Command Line Interface.
******************************************************************************/
static void Help()
{
    cout << "q         quit" << endl;
    cout << "h         show this help message" << endl;
}



/******************************************************************************
 *      Parse
 *      -----
 *      Interprete input from the Command Line.
******************************************************************************/
static bool Parse(const string & input)
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

    case 'h':
    default:
        Help();
        break;
    }

    return true;
}



/******************************************************************************
 *      Shutdown
 *      --------
 *      Stop the AllJoyn Bundled Router and AllJoyn threads.
******************************************************************************/
static void Shutdown() {
    #ifdef ROUTER
        AllJoynRouterShutdown();
    #endif
        AllJoynShutdown();
}



/******************************************************************************
 *      AssetThread
 *      -----------
 *      State, Mode, Control, Logging
******************************************************************************/
void AssetThread(Asset * asset)
{
    while (!done){
        asset->AssetControl();
	this_thread::sleep_for(chrono::milliseconds(1000));
    }
} // END ASSET THREAD



/******************************************************************************
 *      Main
 *      ----
 *      Alljoyn_thread
 *      Asset_thread
******************************************************************************/
int CDECL_CALL main(int argc, char** argv)
{

    if (AllJoynInit() != ER_OK) {
        return EXIT_FAILURE;
    }
#ifdef ROUTER
    if (AllJoynRouterInit() != ER_OK) {
        AllJoynShutdown();
        return EXIT_FAILURE;
    }
#endif

    BusAttachment* bus = NULL;
    bus = new BusAttachment("Asset_provider", true);
    QCC_ASSERT(bus != NULL);
    AboutData aboutData("en");
    AboutObj* aboutObj = new AboutObj(*bus);
    QCC_ASSERT(aboutObj != NULL);

    if (ER_OK != SetupBusAttachment(*bus, aboutData)) {
        delete aboutObj;
        aboutObj = NULL;
        delete bus;
        bus = NULL;
        return EXIT_FAILURE;
    }

    aboutObj->Announce(SERVICE_PORT, aboutData);

    Asset * asset = new Asset(*bus);
    if (ER_OK != bus->RegisterBusObject(*asset)) {
        delete asset;
    }

    aboutObj->Announce(SERVICE_PORT, aboutData);

    //spawn asset thread
    thread thread_1(AssetThread,asset);

    printf("\n\n\n**********\tDMS\t**********\n\n\n");
    Help();
    printf("\n\n\n");
    while (!done) {
        string input;
        cout << "> ";
        getline(cin, input);
        done = !Parse(input);
    }

    //stop program
    thread_1.join();

    delete asset;
    asset = NULL;

    delete aboutObj;
    aboutObj = NULL;

    delete bus;
    bus = NULL;

    Shutdown();
    return EXIT_SUCCESS;
}
