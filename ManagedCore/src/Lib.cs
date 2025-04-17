using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices; // Add this line


namespace DotNetLib
{
    public static class Lib
    {
        private static int s_CallCount = 1;

        [StructLayout(LayoutKind.Sequential)]
        public struct LibArgs
        {
            public IntPtr Message;
            public int Number;
        }
        
        public static int Hello(IntPtr arg, int argLength)
        {
            unsafe
            {

                var c = (char*)(*((ulong*)arg.ToPointer()));

                while (*(c++) != '\0')
                {
                    Console.Write(*c);
                }

                for (int i = 0; i < 10; i++)
                {
                    Console.Write((int)*(c+i));
                    Console.WriteLine();

                }
            }
            Console.WriteLine($"Runtime version: {Environment.Version}");


            if (argLength < System.Runtime.InteropServices.Marshal.SizeOf(typeof(LibArgs)))
            {
                return 1;
            }

            LibArgs libArgs = Marshal.PtrToStructure<LibArgs>(arg);
            Console.WriteLine($"Hello, world! from {nameof(Lib)} [count: {s_CallCount++}]");
            PrintLibArgs(libArgs);
            return 0;
        }

        public delegate void CustomEntryPointDelegate(LibArgs libArgs);
        public static void CustomEntryPoint(LibArgs libArgs)
        {
            Console.WriteLine($"Hello, world! from {nameof(CustomEntryPoint)} in {nameof(Lib)}");
            PrintLibArgs(libArgs);
        }

        [UnmanagedCallersOnly]
        public static void CustomEntryPointUnmanagedCallersOnly(LibArgs libArgs)
        {
            Console.WriteLine($"Hello, world! from {nameof(CustomEntryPointUnmanagedCallersOnly)} in {nameof(Lib)}");
            PrintLibArgs(libArgs);
        }

        private static void PrintLibArgs(LibArgs libArgs)
        {
            string message = 
                Marshal.PtrToStringUni(libArgs.Message)
                ;

            Console.WriteLine($"-- message: {message}");
            Console.WriteLine($"-- number: {libArgs.Number}");
        }
    }
}
