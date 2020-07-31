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
     * supports layers
     */
    public class EntryCold : Entry
    {
        private static ProxyAssLoader loader;
       
        public override void OnAttach() {

            Log.ND_TRACE("C# scripting attached: Cold\n -> Loading assembly");
            AssemblyLocator.InitPaths();
            AssemblyLocator.CheckForModification();
            AssemblyLocator.CopyAssembly();
            if (!File.Exists(AssemblyLocator.DOMAIN_PATH))
            {
                Log.ND_ERROR("Cannot load dll: " + AssemblyLocator.DOMAIN_PATH);
                return;
            }
            loader = new ProxyAssLoader();
            loader.LoadFrom(AssemblyLocator.DOMAIN_PATH);
            loader.LoadLayers();
            loader.AttachLayers();
        }
        public override void OnDetach()
        {
            Log.ND_TRACE("C# scripting detached");
            if (loader != null)
            {
                loader.DetachLayers();
                loader = null;
            }
        }
        public override void OnUpdate()
        {
            if (loader != null)
                loader.UpdateLayers();
        }
        public override List<string> GetLayers()
        {
            var listOut = new List<string>();
            foreach(var layer in loader.layers)
                listOut.Add(layer.GetType().Name);
            return listOut;
        }
    }
}
