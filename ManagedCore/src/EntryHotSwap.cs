
#if no
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
   


}
#endif
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.Loader;

namespace ND
{
    /**
     * Responsible for reloading assembly and supports layers
     */
    public class EntryHotSwap : Entry
    {
        private MyAssemblyLoadContext currentLoadContext;
        private ProxyAssLoaderMarschal loader;
        private int ASS_CHECK_INTERVAL = 60;

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
            currentLoadContext = new MyAssemblyLoadContext();
            loader = new ProxyAssLoaderMarschal();
            loader.LoadFrom(AssemblyLocator.DOMAIN_PATH, currentLoadContext);
            loader.LoadLayers();
            loader.AttachLayers();
        }

        public void UnloadDomain()
        {
            if (loader != null)
            {
                loader.DetachLayers();
                loader.UnloadLayers();
            }

            currentLoadContext?.Unload();
            currentLoadContext = null;
            loader = null;
        }

        public void ReloadDomainThrowing()
        {
            UnloadDomain();
            LoadDomain();
        }

        // File handling
        int searchModificationDivider = 0;
        private void divideCall(Action f)
        {
            searchModificationDivider++;
            if ((searchModificationDivider %= ASS_CHECK_INTERVAL) == 0 && ASS_CHECK_INTERVAL != 0)
                f();
        }

        // Layer
        int counterToCheckAssLoad; // Will wait for a while and then checks if managed was loaded if autoload is enabled

        public override void OnAttach()
        {
            AssemblyLocator.InitPaths();
            counterToCheckAssLoad = ASS_CHECK_INTERVAL * 2;
            Log.ND_TRACE("C# scripting attached: HotSwap");

            if (ASS_CHECK_INTERVAL == 0) // Will be resolved in OnUpdate()
                LoadDomain();
        }

        public override void OnDetach()
        {
            UnloadDomain();
            Log.ND_TRACE("C# scripting detached");
        }

        public override void OnUpdate()
        {
            divideCall(() =>
            {
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
                if (--counterToCheckAssLoad == 0 && loader == null)
                {
                    Log.ND_WARN("Cannot find Managed.dll");
                }
            }
        }

        // Loader
        static Assembly HandleAssemblyResolve(object source, ResolveEventArgs e)
        {
            Console.WriteLine("Cannot load {0} so I am loading", e.Name);
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

    public class MyAssemblyLoadContext : AssemblyLoadContext
    {
        public MyAssemblyLoadContext() : base(isCollectible: true) { }

        protected override Assembly Load(AssemblyName assemblyName)
        {
            // Handle assembly resolution if necessary, or return null to use default behavior
            return null;
        }
    }
}
