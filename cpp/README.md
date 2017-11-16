# AJ_DER
*This file assumes that the user has already successfully built Alljoyn 16.04*

## Setup
1. clone repository into local Dev folder
```
cd $HOME/Dev
git clone github.com/Tylores/AJ_DER
```

2. open appsetup and verify variables are correct, then save and close file.
```
cd AJ_DER
gedit appsetup
```

3. run build_single to set environment variables, compile code, and excecute the EMS/DMS in seperate terminal windows. there is also a build multi script to run 20x DMS for a single EMS to control. 
```
source build_single
source build_multi
```

## AllJoyn
### AllJoynInit()
>	This must be called prior to instantiating or using any AllJoyn functionality.
>	AllJoynShutdown must be called for each invocation of AllJoynInit.

### AllJoynRouterInit()
>	This must be called before using any AllJoyn router functionality.
>	For an application that is a routing node (either standalone or bundled), the complete initialization sequence is:
```c++
AllJoynInit();
AllJoynRouterInit();
```
>	AllJoynRouterShutdown must be called for each invocation of AllJoynRouterInit.

### BusAttachment(const char * applicationName, bool allowRemoteMessages = false, uint32_t concurrency = 4)
>	Construct a BusAttachment.

	* applicationName	Name of the application.
	* allowRemoteMessages	True if this attachment is allowed to receive messages from remote devices.
	* concurrency		The maximum number of concurrent method and signal handlers locally executing.

### SetupBusAttachment(*bus)
#### bus.Start()
>	Start the process of spinning up the independent threads used in the bus attachment, preparing it for action.
> 	This method only begins the process of starting the bus. Sending and receiving messages cannot begin until the bus is Connect()ed.

#### bus.Connect()
>	Connect to a local AllJoyn router.
>	Locate a local AllJoyn router that is compatible with this AllJoyn client's version and connect to the router.

#### BuildInterface(bus)
##### bus.CreateInterface(const char * name, InterfaceDescription *& iface, InterfaceSecurityPolicy secPolicy)
>	Create an interface description with a given name.
>	Typically, interfaces that are implemented by BusObjects are created here. Interfaces that are implemented by remote objects are added automatically by the bus if they are not already present via ProxyBusObject::IntrospectRemoteObject().

>	Interfaces created with this method need to be activated using InterfaceDescription::Activate() once all of the methods, signals, etc have been added to the interface. The interface will be unaccessible (via BusAttachment::GetInterfaces() or BusAttachment::GetInterface()) until it is activated.

	* name				The requested interface name.
	* [out]	iface			Interface description
	* secPolicy			The security policy for this interface
	
##### intf->AddMethod(const char * methodName, const char * inputSig, const char * outSig, const char * argNames, uint8_t annotation = 0, const char * accessPerms = 0)
> 	Add a method call member to the interface

	* methodName	Name of method call member.
	* inputSig	Signature of input parameters or NULL for none.
	* outSig	Signature of output parameters or NULL for none.
	* argNames	Comma separated list of input and then output arg names used in annotation XML.
	* annotation	Annotation flags.
	* accessPerms	Access permission requirements on this call

##### intf->AddProperty(const char * name, const char * signature, uint8_t access)
>	Add a property to the interface

	* name		Name of property.
	* signature	Property type.
	* access	PROP_ACCESS_READ, PROP_ACCESS_WRITE or PROP_ACCESS_RW

##### intf->AddPropertyAnnotation(const qcc::String & p_name, const qcc::String & name, const qcc::String & value)
>	Add an annotation to an existing propertry

	* p_name	Name of the property
	* name		Name of annotation
	* value		Value for the annotation
	
##### intf->Activate()
> 	Activate this interface.
>	An interface must be activated before it can be used. Activating an interface locks the interface so that is can no longer be modified.

###### MsgArg
> 	Class definition for a message arg.
> 	This class deals with the message bus types and the operations on them.

## EMS
### Observer(BusAttachment & bus, const char * mandatoryInterfaces[], size_t numMandatoryInterfaces)
> 	Constructor
> 	the Observer will only discover objects that are announced through About.

	* bus				Bus attachment to which the Observer is attached.
	* mandatoryInterfaces		Set of interface names that a bus object MUST implement to be discoverable by this Observer.
	* numMandatoryInterfaces	number of elements in the mandatoryInterfaces array
	
### RegisterListener(Listener & listener, bool triggerOnExisting = true)
>	Register a listener.

	* listener		the listener to register
	* triggerOnExisting	trigger ObjectDiscovered callbacks for already-discovered objects
	
#### observer->GetFirst();
> 	Get the first proxy object.
> 	The GetFirst/GetNext pair of methods is useful for iterating over all discovered objects. The iteration is over when the proxy object returned by either call is not valid (see ProxyBusObject::IsValid).

#### observer->GetNext(const ObjectId & oid)
> 	Get the next proxy object.
>	The GetFirst/GetNext pair of methods is useful for iterating over all discovered objects. The iteration is over when the proxy object returned by either call is not valid (see ProxyBusObject::IsValid).

	* oid	This method will return the proxy obj
#### proxy.IsValid()
> 	Indicates if this is a valid (usable) proxy bus object. ect immediately following the one with this object id. 
	
#### proxy.GetUniqueName()
> 	Return the remote unique name for this object. 

#### proxy.GetPath()
> 	Return the absolute object path for the remote object. 

#### proxy.MethodCall(const char * ifaceName, const char * methodName, const MsgArg * args, size_t numArgs, uint8_t flags = 0)
> 	Make a fire-and-forget method call from this object.
>	The caller will not be able to tell if the method call was successful or not. This is equivalent to calling MethodCall() with flags == ALLJOYN_FLAG_NO_REPLY_EXPECTED. Because this call doesn't block it can be made from within a signal handler.

	* ifaceName	Name of interface.
	* methodName	Name of method.
	* args		The arguments for the method call (can be NULL)
	* numArgs	The number of arguments
	* flags		Logical OR of the message flags for this method call. The following flags apply to method calls:
	
#### proxy.GetAllProperties(const char * iface, MsgArg & values, uint32_t timeout = DefaultCallTimeout)
> 	Get all properties from an interface on the remote object.
	
	* iface		Name of interface to retrieve all properties from.
	* [out]	values	Property values returned as an array of dictionary entries, signature "a{sv}".
	* timeout	Timeout specified in milliseconds to wait for a reply
	
##### arg.Get(const char * signature, ...)
> 	Matches a signature to the MsArg and if the signature matches unpacks the component values of a MsgArg. 

## DMS
### SessionPortListener 
> 	Abstract base class implemented by AllJoyn users and called by AllJoyn to inform users of session related events

### AboutData aboutData("en")
> 	Create an AboutData class.
> 	The default language will not be set. Use the constructor that takes a default language tag; or set the language using the SetDefaultLanguage member function, CreateFromMsgArg member function or the CreateFromXml member function.
> 	The default language should be specified before any tag that requires localization. These tags are.

	* DeviceName
	* AppName
	* Manufacturer
	* Description

### AboutObj(BusAttachment & bus, AnnounceFlag isAboutIntfAnnounced = UNANNOUNCED)
> 	create a new About class
>	This will also register the About BusObject on the passed in BusAttachment
> 	The AboutObj class is responsible for transmitting information about the interfaces that are available for other applications to use. It also provides application specific information that is contained in the AboutDataListener class

#### aboutObj->Announce(SERVICE_PORT, aboutData)
> 	This is used to send the Announce signal.
> 	It announces the list of all interfaces available at given object paths as well as the announced fields from the AboutData.
>	This method will automatically obtain the Announced ObjectDescription from the BusAttachment that was used to create the AboutObj. Only BusObjects that have marked their interfaces as announced and are registered with the BusAttachment will be announced.

	* sessionPort	the session port the interfaces can be connected with
	* aboutData	the AboutDataListener that contains the AboutData for this announce signal.

### BusObject
> 	Message Bus Object base class.

#### bus.GetInterface(const char * name)
> 	Retrieve an existing activated InterfaceDescription.

	* name	Interface name
	
#### AddInterface(const InterfaceDescription & iface, AnnounceFlag isAnnounced = UNANNOUNCED )
> 	Add an interface to this object.
> 	If the interface has properties this will also add the standard property access interface. An interface must be added before its method handlers can be added. Note that the Peer interface (org.freedesktop.DBus.peer) is implicit on all objects and cannot be explicitly added, and the Properties interface (org.freedesktop,DBus.Properties) is automatically added when needed and cannot be explicitly added.

	* iface		The interface to add
	* isAnnounced	This interface should be part of the Announce signal UNANNOUNCED - this interface will not be part of the Announce signal ANNOUNCED - this interface will be part of the Announce signal.
	
#### AddMethodHandlers(const InterfaceDescription::Member * member, MessageReceiver::MethodHandler handler, void *  	context = NULL)
> 	Add a method handler to this object.
> 	The interface for the method handler must have already been added by calling AddInterface().

	* member	Interface member implemented by handler.
	* handler	Method handler.
	* context	An optional context. This is mainly intended for implementing language bindings and should normally be NULL.

#### Get(const char * ifcName, const char * propName, MsgArg & val)
> 	Handle a bus request to read a property from this object.
> 	BusObjects that implement properties should override this method. The default version simply returns ER_BUS_NO_SUCH_PROPERTY.

	* ifcName	Identifies the interface that the property is defined on
	* propName	Identifies the property to get
	* [out]	val	Returns the property value. The type of this value is the actual value type. 

#### bus->RegisterBusObject(BusObject & obj, bool secure = false)
> 	Register a BusObject.

	* obj		BusObject to register.
	* secure	true if authentication is required to access this object.

##### arg.Set(const char * signature, ...)
> 	Set value of a message arg from a signature and a list of values.
> 	Note that any values or MsgArg pointers passed in must remain valid until this MsgArg is freed.

