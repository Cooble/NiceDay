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
    /**
     * Responsible for reloading assembly and supports layers
     */
    public class EntryHotSwap:Entry
    {
       

        private AppDomain currentDomain;
        private ProxyAssLoaderMarschal loader;
        private int ASS_CHECK_INTERVAL =60;
        // domain
        public override void SetAssemblyOutsideMode(bool b) => AssemblyLocator.ENABLE_OUTSIDE_SEARCH = b;
        public override void SetAssemblyLookupInterval(int i) => ASS_CHECK_INTERVAL = i;
        public override void ReloadAssembly()
        {
            try
            {
                ReloadDomainThrowing();
            }
            catch (Exception e)
            {
                Log.ND_ERROR("Failed to reload assembly due to error:");
                Log.ND_ERROR(e.ToString());
            }
        }
        public void LoadDomain()
        {
            var setup = new AppDomainSetup();
            setup.ApplicationBase =AssemblyLocator.ROOT_PATH;
            //ND.Log.ND_INFO("new appdomain in " + setup.ApplicationBase);

            currentDomain = AppDomain.CreateDomain(AssemblyLocator.DOMAIN_NAME, null, setup);
            //ND.Log.ND_INFO("New Domain created: " + currentDomain.ToString());

            currentDomain.AssemblyResolve += HandleAssemblyResolve;
            loader = (ProxyAssLoaderMarschal)currentDomain.CreateInstanceAndUnwrap(typeof(ProxyAssLoaderMarschal).Assembly.FullName, typeof(ProxyAssLoaderMarschal).FullName);
            loader.LoadFrom(AssemblyLocator.DOMAIN_PATH);

           // Log.ND_TRACE("Host domain: " + AppDomain.CurrentDomain.FriendlyName);
           // Log.ND_TRACE("child domain: " + currentDomain.FriendlyName);
            //Log.ND_TRACE("Application base is: " + currentDomain.SetupInformation.ApplicationBase);

            loader.LoadLayers();
            loader.AttachLayers();
        }
        public void UnloadDomain()
        {
            if (currentDomain != null){
                if (loader != null) {
                    loader.DetachLayers();
                    loader.UnloadLayers();
                }
               // Log.ND_TRACE("Unloading domain: " + currentDomain.FriendlyName);
                AppDomain.Unload(currentDomain);
            }
            currentDomain = null;
            loader = null;
        }
        public void ReloadDomainThrowing()
        {
            UnloadDomain();
            LoadDomain();
        }

        //file handling
      
        int searchModificationDivider = 0;
        private void divideCall(Action f)
        {
            searchModificationDivider++;
            if ((searchModificationDivider %= ASS_CHECK_INTERVAL) == 0 && ASS_CHECK_INTERVAL != 0)
                f();
        }
        
      

        //layer
        int counterToCheckAssLoad;//will wait for a while and then checks if managed was loaded if autoload is enabled

        public override void OnAttach()
        {
            AssemblyLocator.InitPaths();
            counterToCheckAssLoad = ASS_CHECK_INTERVAL*2;
            Log.ND_TRACE("C# scripting attached: HotSwap");
            
            if (ASS_CHECK_INTERVAL == 0)//will be resolved in OnUpdate()
                LoadDomain();
        }
        public override void OnDetach()
        {
            if (currentDomain != null)
                AppDomain.Unload(currentDomain);
            Log.ND_TRACE("C# scripting detached");
        }
        public override void OnUpdate()
        {
            divideCall(() => {
                if (AssemblyLocator.CheckForModification())
                {
                    Log.ND_INFO("Found newer dll - > copying");
                    UnloadDomain();
                    AssemblyLocator.CopyAssembly();
                    LoadDomain();
                }
            });
            if (loader != null)
                loader.UpdateLayers();

            if (counterToCheckAssLoad > 0)
            {
                if (--counterToCheckAssLoad == 0 && loader==null)
                {
                    Log.ND_WARN("Cannot find Managed.dll");
                }
            }


        }
        //loader
        static Assembly HandleAssemblyResolve(object source, ResolveEventArgs e)
        {
            Console.WriteLine("Cannot load {0} so i am loading", e.Name);
            return Assembly.Load(e.Name);
        }
        public override List<string> GetLayers()
        {
            var listOut = new List<string>();
            if (loader == null)
                return listOut;
            foreach (var layer in loader.layers)
                listOut.Add(layer.GetType().Name);
            return listOut;
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
        private static bool flagIsOutsideFresher=false;

        private static DateTime lastNewDomainModification=new DateTime(0);

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
            } else 
                OUTSIDE_MANAGED_DOMAIN_PATH = OUTSIDE_MANAGED_DIR_PATH +"\\"+ DOMAIN_NAME;
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

            Log.ND_COPY_FILE(flagIsOutsideFresher?OUTSIDE_MANAGED_DOMAIN_PATH:NEW_DOMAIN_PATH, DOMAIN_PATH);//using this approach due to some pesky security errors in c#
            updateModificationStamp();
        }
        //check if assembly changed in exe folder
        public static bool CheckForModification()
        {
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
