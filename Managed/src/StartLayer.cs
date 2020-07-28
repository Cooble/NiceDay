using System;
using System.Runtime.CompilerServices;
using ND;
public class TestingLayer:Layer
{
    public override void OnAttach() 
    {
        Log.ND_TRACE("TestingLayer attached");
    }
    public override void OnDetach()
    {
        Log.ND_TRACE("TestingLayer detached");
    }
}
