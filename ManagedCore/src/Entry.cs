using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;

namespace ND
{

    /*
     * Supports layers
     */
    public abstract class Entry
    {
        public static Entry Instance;

        public static Entry Init(bool hotSwapEnable)
        {
            if (hotSwapEnable)
                Instance = new EntryHotSwap();
            else Instance = new EntryCold();
            //Log.ND_TRACE("Initialized Entry as " + (hotSwapEnable ? "EntryHotSwap" : "EntryCold"));
            return Instance;
        }
        /** 
         * how frequently check for new assembly in exe directory
         * 0 means never (to reload you need to call ReloadAssembly())
        */
        public virtual void SetAssemblyLookupInterval(int eachNTicks) => Log.ND_WARN("Calling unsupported method");
        public virtual void ReloadAssembly() => Log.ND_WARN("Calling unsupported method");
        public virtual void OnAttach() { }
        public virtual void OnDetach() { }
        public virtual void OnUpdate() { }

        public  abstract List<string> GetLayers();
        public int GetLayersSize() { return GetLayers().Count; }


      
    }
}
