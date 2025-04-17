using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
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
            Console.WriteLine("Initialized Entry as " + (hotSwapEnable ? "EntryHotSwap" : "EntryCold"));
            return Instance;
        }
        public static void Test(){}
        /** 
         * how frequently check for new assembly in exe directory
         * 0 means never (to reload you need to call ReloadAssembly())
        */
        public virtual void SetAssemblyLookupInterval(int eachNTicks) => Log.ND_WARN("Calling unsupported method");

        // whether the managed.dll should be searched for in project Managed in current cmake project structure (certainly not for Dist config)
        // if set to false you need to manually copy dll to exe folder to be automaticaly reloaded
        public virtual void SetAssemblyOutsideMode(bool outside) => Log.ND_WARN("Calling unsupported method");

        public virtual void ReloadAssembly() => Log.ND_WARN("Calling unsupported method");
        public virtual void OnAttach() { }
        public virtual void OnDetach() { }
        public virtual void OnUpdate() { }

        public  abstract List<string> GetLayers();
        public int GetLayersSize() => GetLayers() == null ? -1 : GetLayers().Count;
    }

    public static class EntryCaller
    {
        [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
        public static void Init(int hotswap)
        {
            Entry.Init(hotswap!=0);
        }

        [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
        public static void OnAttach()
        {
            try
            {
                Entry.Instance.OnAttach();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception in OnAttach: " + e);
            }
        }
        [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]

        public static void OnDetach()
        {
            try
            {
                Entry.Instance.OnDetach();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception in OnDetach: " + e);
            }
        }
        [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
        public static void OnUpdate()
        {
            try
            {
                Entry.Instance.OnUpdate();
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception in OnUpdate: " + e);
            }
        }
    }

    public static class AssemblyLocator
    {
        public static string DOMAIN_NAME_RELEASE = "Managed.dll";
        public static string DOMAIN_NAME_DEBUG = "Managedd.dll";

        public static string ROOT_PATH = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        public static string DOMAIN_DIR_PATH = ROOT_PATH + @"\attachedDll";
        public static string DOMAIN_NAME;
        public static string NEW_DOMAIN_PATH;//dll put into root path to be used
        public static string DOMAIN_PATH;
        public static string OUTSIDE_MANAGED_DIR_PATH;
        public static string OUTSIDE_MANAGED_DOMAIN_PATH;
        public static bool ENABLE_OUTSIDE_SEARCH = false;
        private static bool flagIsOutsideFresher = false;

        private static DateTime lastNewDomainModification = new DateTime(0);

        private static void updateModificationStamp()
        {
            if (File.Exists(DOMAIN_PATH))
                lastNewDomainModification = File.GetLastWriteTime(DOMAIN_PATH);
            else lastNewDomainModification = new DateTime(0);
        }
        private static bool IsFileLocked(FileInfo file)
        {
            try
            {
                using (FileStream stream = file.Open(FileMode.Open, FileAccess.Read, FileShare.None))
                {
                    stream.Close();
                }
            }
            catch (Exception) { return true; }
            return false;
        }
        public static void InitPaths()
        {
            DOMAIN_NAME = Log.ND_CURRENT_CONFIG() == "ND_DEBUG" ? DOMAIN_NAME_DEBUG : DOMAIN_NAME_RELEASE;
            NEW_DOMAIN_PATH = ROOT_PATH + "\\" + DOMAIN_NAME;//dll put into root path to be used
            DOMAIN_PATH = DOMAIN_DIR_PATH + "\\" + DOMAIN_NAME;


            OUTSIDE_MANAGED_DIR_PATH = Directory.GetParent(Directory.GetParent(ROOT_PATH).FullName).FullName + @"\Managed\" + new DirectoryInfo(ROOT_PATH).Name;
            if (!Directory.Exists(OUTSIDE_MANAGED_DIR_PATH))
            {
                Log.ND_ERROR("Cannot find dll in project Managed, dir " + OUTSIDE_MANAGED_DIR_PATH + " doesn't exist!");
                OUTSIDE_MANAGED_DIR_PATH = "";
            }
            else
                OUTSIDE_MANAGED_DOMAIN_PATH = OUTSIDE_MANAGED_DIR_PATH + "\\" + DOMAIN_NAME;
            if (!Directory.Exists(DOMAIN_DIR_PATH))
            {
                Log.ND_INFO("dir not exist " + DOMAIN_DIR_PATH);
                Directory.CreateDirectory(DOMAIN_DIR_PATH);
            }
            else Log.ND_INFO("dir exist " + DOMAIN_DIR_PATH);


            if (File.Exists(NEW_DOMAIN_PATH))
                Log.ND_COPY_FILE(NEW_DOMAIN_PATH, DOMAIN_PATH);
        }
        public static void CopyAssembly()
        {
            if (!Directory.Exists(DOMAIN_DIR_PATH))
                Directory.CreateDirectory(DOMAIN_DIR_PATH);

            Log.ND_COPY_FILE(flagIsOutsideFresher ? OUTSIDE_MANAGED_DOMAIN_PATH : NEW_DOMAIN_PATH, DOMAIN_PATH);//using this approach due to some pesky security errors in c#
            updateModificationStamp();
        }
        //check if assembly changed in exe folder
        public static bool CheckForModification()
        {
            //Log.ND_INFO("checking for modification of "+NEW_DOMAIN_PATH);
            if (File.Exists(NEW_DOMAIN_PATH))
            {
                DateTime current = File.GetLastWriteTime(NEW_DOMAIN_PATH);
                if (current > lastNewDomainModification && !IsFileLocked(new FileInfo(NEW_DOMAIN_PATH)))
                {
                    flagIsOutsideFresher = false;
                    return true;
                }
            }
            if (!ENABLE_OUTSIDE_SEARCH)
                return false;
            if (File.Exists(OUTSIDE_MANAGED_DOMAIN_PATH))
            {
                DateTime current = File.GetLastWriteTime(OUTSIDE_MANAGED_DOMAIN_PATH);
                if (current > lastNewDomainModification && !IsFileLocked(new FileInfo(OUTSIDE_MANAGED_DOMAIN_PATH)))
                {
                    flagIsOutsideFresher = true;
                    return true;
                }
            }
            return false;
        }

    }
}
