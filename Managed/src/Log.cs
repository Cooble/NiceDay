using System;
using System.Runtime.CompilerServices;


public class Log
{
    // print stack trace before message
    public static bool ShowStackTrace = true;
    // don't print method name
    public static bool Shortened = true;

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    private static extern void nd_trace(string message);
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    private static extern void nd_info(string message);
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    private static extern void nd_warn(string message);
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    private static extern void nd_error(string message);

    private static string buildMessage(string fileName,string memberName,int line,string message)
    {
        if (!ShowStackTrace)
            return message;
        fileName = fileName.Replace('\\', '/');
        return "[" + fileName.Substring(fileName.LastIndexOf("/") + 1) + (Shortened?"":(":" + memberName))+ ":" + line + "]: " + message;

    }
    public static void ND_TRACE(string message,
       [CallerFilePath] string fileName = "",
       [CallerMemberName] string memberName = "",
       [CallerLineNumber] int line = 0)
    {
        nd_trace(buildMessage(fileName, memberName, line, message));
    }
    public static void ND_INFO(string message,
      [CallerFilePath] string fileName = "",
      [CallerMemberName] string memberName = "",
      [CallerLineNumber] int line = 0)
    {
        nd_info(buildMessage(fileName, memberName, line, message));
    }
    public static void ND_WARN(string message,
     [CallerFilePath] string fileName = "",
     [CallerMemberName] string memberName = "",
     [CallerLineNumber] int line = 0)
    {
        nd_warn(buildMessage(fileName, memberName, line, message));
    }
    public static void ND_ERROR(string message,
   [CallerFilePath] string fileName = "",
   [CallerMemberName] string memberName = "",
   [CallerLineNumber] int line = 0)
    {
        nd_error(buildMessage(fileName, memberName, line, message));
    }



}