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
        private static string ROOT_PATH = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)+"\\";
        private static string DOMAIN_NAME = "Managedd.dll";
        private static string NEW_DOMAIN_PATH = ROOT_PATH+DOMAIN_NAME;//dll put into root path to be used
        private static string DOMAIN_DIR_PATH = ROOT_PATH+ @"attachedDll\";
        private static string DOMAIN_PATH = DOMAIN_DIR_PATH + DOMAIN_NAME;
        private static string OUTSIDE_MANAGED_DIR_PATH;

        private AppDomain currentDomain;
        private ProxyAssLoaderMarschal loader;
        private int ASS_CHECK_INTERVAL =60;
        private bool CHECK_OUTSIDE = true;
        // domain

        private static void updatePaths()
        {
            var folderName = new DirectoryInfo(ROOT_PATH).Name;
          
            OUTSIDE_MANAGED_DIR_PATH = Directory.GetParent(Directory.GetParent(Directory.GetParent(ROOT_PATH).FullName).FullName).FullName + @"\Managed\" + folderName+"\\";
            if (!Directory.Exists(OUTSIDE_MANAGED_DIR_PATH))
            {
                Log.ND_ERROR("Cannot find dll in project Managed, dir "+OUTSIDE_MANAGED_DIR_PATH+" doesn't exist!");
                OUTSIDE_MANAGED_DIR_PATH = "";
            }
        }
        public override void SetAssemblyLookupInterval(int i) => ASS_CHECK_INTERVAL = i;
        public override void ReloadAssembly()
        {
            ReloadDomain();
        }
        public void LoadDomain()
        {
            CopyDomain();

            var setup = new AppDomainSetup();
            setup.ApplicationBase = ROOT_PATH;
            //ND.Log.ND_INFO("new appdomain in " + setup.ApplicationBase);

            currentDomain = AppDomain.CreateDomain(DOMAIN_NAME, null, setup);
            //ND.Log.ND_INFO("New Domain created: " + currentDomain.ToString());

            currentDomain.AssemblyResolve += HandleAssemblyResolve;
            loader = (ProxyAssLoaderMarschal)currentDomain.CreateInstanceAndUnwrap(typeof(ProxyAssLoaderMarschal).Assembly.FullName, typeof(ProxyAssLoaderMarschal).FullName);
            loader.LoadFrom(DOMAIN_PATH);

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
        public void ReloadDomain()
        {
            try
            {
                ReloadDomainThrowing();
            }
            catch (Exception e)
            {
                Console.WriteLine("Failed to reload due to error:");
                Console.WriteLine(e.ToString());
            }
        }

        //file handling
        private void CopyDomain()
        {
            if (!Directory.Exists(DOMAIN_DIR_PATH))
                Directory.CreateDirectory(DOMAIN_DIR_PATH);
            File.Copy(NEW_DOMAIN_PATH, DOMAIN_PATH, true);
        }
        private bool IsFileLocked(FileInfo file)
        {
            try{
                using (FileStream stream = file.Open(FileMode.Open, FileAccess.Read, FileShare.None)){
                    stream.Close();
                }
            }
            catch (IOException)
            {
                //the file is unavailable because it is:
                //still being written to
                //or being processed by another thread
                //or does not exist (has already been processed)
                return true;
            }
            return false;
        }
       
        int searchModificationDivider = 0;
        private void divideCall(Action f)
        {
            searchModificationDivider++;
            if ((searchModificationDivider %= ASS_CHECK_INTERVAL) == 0 && ASS_CHECK_INTERVAL != 0)
                f();
        }
        
        DateTime lastAssModification;
        private void CheckForModification()
        {
            if (lastAssModification != null)
                {
                    DateTime current = File.GetLastWriteTime(NEW_DOMAIN_PATH);
                    if (current > lastAssModification && !IsFileLocked(new FileInfo(NEW_DOMAIN_PATH)))
                    {
                        Log.ND_INFO("Found newer version of dll -> reloading");
                        lastAssModification = current;
                        ReloadDomain();
                    }
            }
            else lastAssModification = File.GetLastWriteTime(NEW_DOMAIN_PATH);
        }
        static DateTime lastOutside;
        //check if project called Managed has newer version and copies it to exe folder
        private void CheckForModificationOutside()
        {
            if (!File.Exists(OUTSIDE_MANAGED_DIR_PATH + DOMAIN_NAME))
                return;
                if (lastOutside != null)
                {
                    DateTime current = File.GetLastWriteTime(OUTSIDE_MANAGED_DIR_PATH+DOMAIN_NAME);

                    if (current > lastOutside)
                    {
                        //Log.ND_TRACE("Found newer version of dll -> moving");
                        lastOutside = current;
                        Log.ND_COPY_FILE(OUTSIDE_MANAGED_DIR_PATH+DOMAIN_NAME, NEW_DOMAIN_PATH);//using this approach due to some pesky security errors in c#
                        CheckForModification();
                }
                }
           else lastOutside = File.GetLastWriteTime(NEW_DOMAIN_PATH);
            
        }

        //layer

        public override void OnAttach()
        {
            updatePaths();
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
            if(CHECK_OUTSIDE)//look only in project Managed
                divideCall(CheckForModificationOutside);
            else//look only in exe folder
                divideCall(CheckForModification);
            if (loader != null)
                loader.UpdateLayers();

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
            foreach (var layer in loader.layers)
                listOut.Add(layer.GetType().Name);
            return listOut;
        }
    }
  

}
