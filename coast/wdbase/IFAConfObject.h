/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IFACONFOBJECT_H
#define _IFACONFOBJECT_H

#include "config_wdbase.h"

#include "IFAObject.h"
#include "LookupInterface.h"
#include "Anything.h"

class Registry;
class InstallerPolicy;
class TerminationPolicy;

//---- RegisterableObject ----------------------------------------------------------
//!defines api to support registerable objects; objects registered with a name in a category
class EXPORTDECL_WDBASE RegisterableObject : public NamedObject
{
public:
	//!name object constructor
	RegisterableObject(const char *name);

	//! register api; objects can be registered with a category and a name
	virtual void Register(const char *name, const char *category);

	//!remove object from the registry
	virtual void Unregister(const char *name, const char *category);

	//! object marked by static initializer object as statically allocated; this prevents this object from deletion by the terminators
	virtual void MarkStatic() {
		fStaticallyInitialized = true;
	}

	//! query static initialized flag
	virtual bool IsStatic()	  {
		return fStaticallyInitialized;
	}

	//! implements API for naming support
	virtual void SetName(const char *str)	{
		fName = str;
	}

	//! implements API for naming support
	virtual bool GetName(String &str) const {
		str = fName;
		return true;
	}

	//! implements API for getting name
	virtual const char *GetName() const {
		return (const char *)fName;
	}

	// support for registerable named objects
	// this method gets called during initialization

	//!installs additional objects according to installerspec and installer policy into category
	static bool Install(const ROAnything installerSpec, const char *category, InstallerPolicy *ip);
	//!terminates category removing all objects from the registry as implemented by the termination policy
	static bool Terminate(const char *category, TerminationPolicy *tp);
	//!terminates category removing only the objects which are not installed by the static initializers to allow reinitialization
	static bool ResetTerminate(const char *category, TerminationPolicy *terminator);
	//!terminates category removing only the objects which are not installed by the static initializers and installs again with installerSpec
	static bool Reset(const ROAnything installerSpec, const char *category, InstallerPolicy *installer, TerminationPolicy *terminator);

	friend class WDModuleTest;

protected:
	//! object name represented as string
	String fName;

	//! flag to know if object is allocated by static registerer
	bool   fStaticallyInitialized;

	//!sets flag to reset cache
	static void ResetCache(bool resetCache);

	//!flag that triggers reset of registry cache
	static bool fgResetCache;

private:
	//!do not use
	RegisterableObject();
	//!do not use
	RegisterableObject(const RegisterableObject &);
	//!do not use
	RegisterableObject &operator=(const RegisterableObject &);
};

//!objects are generated as static variables by macro RegisterObject; installs RegisterableObject r with name in category
class EXPORTDECL_WDBASE RegisterableObjectInstaller
{
public:
	//!installs r into category with name; caches r into fObject
	RegisterableObjectInstaller(const char *name, const char *category, RegisterableObject *r);
	//!deletes fObject
	~RegisterableObjectInstaller();
protected:
	//!the object that was installed in the registry
	RegisterableObject *fObject;
	//!the name of the installed object for debugging purposes
	String fName;
	//!the category of the installed object for debugging purposes
	String fCategory;
};

//!creates static variable of type RegisterableObjectInstaller
#define RegisterObject(name, category) \
	static RegisterableObjectInstaller _NAME3_(name,category,Registerer) (_QUOTE_(name), _QUOTE_(category), new name(_QUOTE_(name)))

//!creates static variable of type RegisterableObjectInstaller with a short name
#define RegisterShortName(sname, name, instname) \
	static RegisterableObjectInstaller _NAME3_(sname,instname,Registerer) (_QUOTE_(sname), _QUOTE_(instname), new name(_QUOTE_(sname)))

//!creates definition for a Find method for category
#define RegCacheDef(category) 										\
	static category *_NAME2_(Find, category)(const char *)

//!creates implementation for a Find method for category
#define RegCacheImpl(category) 																	\
	_NAME1_(category) *_NAME1_(category)::_NAME2_(Find, category)(const char *name) 			\
	{ 																							\
		/* StartTrace(_NAME1_(category)._NAME2_(Find, category))	*/							\
		static Registry *fgRegistry= 0;															\
																								\
		if ( !fgRegistry || RegisterableObject::fgResetCache ) fgRegistry= Registry::GetRegistry(_NAME1_(_QUOTE_(category))); 				\
		_NAME1_(category) *catMember = 0;															\
		if (name)																				\
		{																						\
			/* Trace("Looking for <" << name << "> in category:<" << _QUOTE_(category) << ">"); */\
			catMember= SafeCast(fgRegistry->Find(name),_NAME1_(category));/*(_NAME1_(category) *)fgRegistry->Find(name);	*/							\
		}																						\
		return catMember;																				\
	}

//---- NotCloned ----------------------------------------------------------
//!RegisterableObject as singleton; only aliases to the same object are installed
//! NotCloned provides a special clone method that just returns this
//! therefore it installes simple aliases ( entries in the registry, that
//! access the same object under different names eg. HTMLTemplateRenderer and
//! HTML )
class EXPORTDECL_WDBASE NotCloned : public RegisterableObject
{
public:
	//!named object constructor
	NotCloned(const char *name) : RegisterableObject(name) { }
	//!returns pseudo clone this; provides singleton
	IFAObject *Clone() const {
		NotCloned *nonconst_this = (NotCloned *) this;
		return nonconst_this;
	}

private:
	//!do not use
	NotCloned();
	//!do not use
	NotCloned(const NotCloned &);
	//!do not use
	NotCloned &operator=(const NotCloned &);

};

//---- ConfNamedObject ----------------------------------------------------------
/*! configurable object; api for configuration support with default implementation
ConfNamedObject provides a protocol for handling configuration data
it already provides a default implementation that is sufficient
for most classes ( eg. Role and Page ) */
class EXPORTDECL_WDBASE ConfNamedObject : public RegisterableObject, public virtual LookupInterface
{
public:
	//!named object constructor
	ConfNamedObject(const char *);
	//!does nothing
	~ConfNamedObject();

	/*! Public api to create a cloned object. The real work is delegated to the virtual Do.. method. After successful cloning, the objects configuration data gets loaded if possible.
		\param category Name of the category in which the objects configuration will be stored in the Cache
		\param name Name of the clone
		\param forceReload If set to true, the objects configuration will always be loaded for new
		\return New object clone with initialized configuration */
	ConfNamedObject *ConfiguredClone(const char *category, const char *name, bool forceReload = false);

	//! overriden to load config for statically registered objects
	virtual void Register(const char *name, const char *category);

	//!check if configuration is loaded; load it if not done or forceReload= true
	//! check for configuration data, assumes that fName is valid and category is
	//! provided with the correct value
	//! if the configuration data is not already loaded it tries to do so
	virtual bool CheckConfig(const char *category, bool forceReload = false);

protected:
	//!the configuration of this object
	ROAnything fConfig;

	/*! Creates a new object through cloning. Generates a cloned object with a different name.
		\param category Name of the category in which the objects configuration will be stored in the Cache
		\param name Name of the clone
		\param forceReload If set to true, the objects configuration will always be loaded for new
		\return New object clone, name is set but configuration will be initialized in calling method */
	virtual ConfNamedObject *DoConfiguredClone(const char *category, const char *name, bool forceReload);

	//!generate the config file name
	//! generate the config file name (without extension, which is assumed to be any)
	//! out of category and objName
	//! the default implementation just takes the objName
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

	//!load the configuration anything
	//! load an anything and make it available by storing a reference in fConfig
	//! the default implementation uses the cache handler
	//! if you provide your own implementation be careful about the lifetime of
	//! the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);

	//!implement the LookupInterface
	//! method to subclass if the lookup behaviour shall deviate from the standard
	//! implementation (i.e. allow more Anys to be searched, hierarchical, etc)
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

private:
	//!do not use
	ConfNamedObject();
	//!do not use
	ConfNamedObject(const ConfNamedObject &);
	//!do not use
	ConfNamedObject &operator=(const ConfNamedObject &);
};

//---- HierarchConfNamed ----------------------------------------------------------

//!configurable object; api for configuration support with hierachical object relationships (inheritance)
//!implements inheritance relationship of configurations through super objects
class EXPORTDECL_WDBASE HierarchConfNamed : public ConfNamedObject
{
public:
	//!named object constructor
	HierarchConfNamed(const char *);
	virtual ~HierarchConfNamed() {}

	//! hierarchical relationship API; set super object
	virtual void SetSuper(HierarchConfNamed *super);

	//! hierarchical relationship API; get super object
	virtual HierarchConfNamed *GetSuper() const;

protected:
	/*! Creates a new object through cloning. Generates a cloned object with a different name.
		\param category Name of the category in which the objects configuration will be stored in the Cache
		\param name Name of the clone
		\param forceReload If set to true, the objects configuration will always be loaded for new
		\return New object clone, name is set but configuration will be initialized in calling method */
	virtual ConfNamedObject *DoConfiguredClone(const char *category, const char *name, bool forceReload);

	//!implement lookup of immutable configuration
	virtual bool DoLookup(const char *key, class ROAnything &resultconst, char delim, char indexdelim) const;

	//!pointer to super object
	HierarchConfNamed *fSuper;

private:
	//!do not use
	HierarchConfNamed();
	//!do not use
	HierarchConfNamed(const HierarchConfNamed &);
	//!do not use
	HierarchConfNamed &operator=(const HierarchConfNamed &);
};

#endif
