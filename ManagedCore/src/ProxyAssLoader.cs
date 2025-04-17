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
    public class ProxyAssLoader
    {
        private Assembly ass;
        public List<Layer> layers = new List<Layer>();
        public void Load(string path)
        {
            ValidatePath(path);

            ass = Assembly.Load(path);
        }

        public void LoadFrom(string path)
        {
            ValidatePath(path);

            ass = Assembly.LoadFrom(path);
        }

        private void ValidatePath(string path)
        {
            if (path == null) throw new ArgumentNullException(nameof(path));
            if (!System.IO.File.Exists(path))
                throw new ArgumentException($"path \"{path}\" does not exist");
        }
        public void LoadLayers()
        {
            try
            {
                // ND.Log.ND_INFO("got " + ass.GetType("ManagedStart").GetMethod("Start").Invoke(null, null));

                var lays = ass.GetTypes().Where(x => x.BaseType == typeof(Layer));
                string layersList = " ";
                foreach (var tempClass in lays)
                {
                    // var curInsance = ass.CreateInstance(tempClass);
                    layersList += ", " + tempClass.Name;
                    var curInsance = Activator.CreateInstance(tempClass);
                    layers.Add((Layer)curInsance);

                    // using reflection I will be able to run the method as:
                    // curInsance.GetType().GetMethod("Run").Invoke(curInsance, null);
                }
                Log.ND_TRACE("Loaded C# layers: " + layersList.Substring(1));
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

        }
        public void AttachLayers()
        {
            foreach (Layer l in layers)
                l.OnAttach();
        }
        public void DetachLayers()
        {
            foreach (Layer l in layers)
                l.OnDetach();
        }
        public void UnloadLayers()
        {
            layers.Clear();
        }
        public void UpdateLayers()
        {
            foreach (Layer l in layers)
                l.OnUpdate();
        }
    }
    public class ProxyAssLoaderMarschal: MarshalByRefObject
    {
        private Assembly ass;
        public List<Layer> layers = new List<Layer>();
        public void Load(string path)
        {
            ValidatePath(path);

            ass = Assembly.Load(path);
        }

        public void LoadFrom(string path)
        {
            ValidatePath(path);

            ass = Assembly.LoadFrom(path);
        }

        private void ValidatePath(string path)
        {
            if (path == null) throw new ArgumentNullException(nameof(path));
            if (!System.IO.File.Exists(path))
                throw new ArgumentException($"path \"{path}\" does not exist");
        }
        public void LoadLayers()
        {
            try
            {
                // ND.Log.ND_INFO("got " + ass.GetType("ManagedStart").GetMethod("Start").Invoke(null, null));

                var lays = ass.GetTypes().Where(x => x.BaseType == typeof(Layer));
                string layersList="";
                foreach (var tempClass in lays)
                {
                    // var curInsance = ass.CreateInstance(tempClass);
                    layersList+=", "+tempClass.Name;
                    var curInsance = Activator.CreateInstance(tempClass);
                    layers.Add((Layer)curInsance);

                    // using reflection I will be able to run the method as:
                    // curInsance.GetType().GetMethod("Run").Invoke(curInsance, null);
                }
                Log.ND_TRACE("Loaded C# layers: " + layersList.Substring(1));
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }

        }
        public void AttachLayers()
        {
            foreach (Layer l in layers)
                l.OnAttach();
        }
        public void DetachLayers()
        {
            foreach (Layer l in layers)
                l.OnDetach();
        }
        public void UnloadLayers()
        {
            layers.Clear();
        }
        public void UpdateLayers()
        {
            foreach (Layer l in layers)
                l.OnUpdate();
        }
    }
}
#endif

using System.Collections.Generic;
using System.Reflection;
using System.Runtime.Loader;
using System;
using System.Linq;

namespace ND
{
    public class ProxyAssLoaderMarschal : MarshalByRefObject
    {
        private Assembly ass;
        public List<Layer> layers = new();

        public void Load(string path)
        {
            ValidatePath(path);
            ass = Assembly.Load(path);
        }

        public void LoadFrom(string path, AssemblyLoadContext context)
        {
            ValidatePath(path);
            ass = context.LoadFromAssemblyPath(path);
        }

        private void ValidatePath(string path)
        {
            if (path == null) throw new ArgumentNullException(nameof(path));
            if (!System.IO.File.Exists(path))
                throw new ArgumentException($"path \"{path}\" does not exist");
        }

        public void LoadLayers()
        {
            try
            {
                var lays = ass.GetTypes().Where(x => x.BaseType == typeof(Layer));
                string layersList = " ";
                foreach (var tempClass in lays)
                {
                    layersList += ", " + tempClass.Name;
                    var curInstance = Activator.CreateInstance(tempClass);
                    layers.Add((Layer)curInstance);
                }
                Log.ND_TRACE("Loaded C# layers: " + layersList.Substring(1));
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public void AttachLayers()
        {
            foreach (Layer l in layers)
                l.OnAttach();
        }

        public void DetachLayers()
        {
            foreach (Layer l in layers)
                l.OnDetach();
        }

        public void UnloadLayers()
        {
            layers.Clear();
        }

        public void UpdateLayers()
        {
            foreach (Layer l in layers)
                l.OnUpdate();
        }
    }
}
