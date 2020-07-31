using System;
using System.Reflection;
using System.Runtime.InteropServices;

namespace ND
{
    public abstract class Layer
    {
        public virtual void OnAttach() { }
        public virtual void OnDetach() { }
        public virtual void OnUpdate() { }

    }
}
