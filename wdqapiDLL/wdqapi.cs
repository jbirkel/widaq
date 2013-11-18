#region File Header
// ----------------------------------------------------------------------------
// WDQAPI C# Header
//
//   C# version of the WDQAPI "C" language API
// ----------------------------------------------------------------------------
#endregion

using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Globalization;

using System.Net;

using System.Runtime.InteropServices; // DllImport

//using jhblib;

namespace WdqApi 
{
   public class wdqapi
   {
      const int NUM_WEP_KEYS_MAX  =   4;
      //const int WPA_PSK_KEY_SIZE  =  32;            
      //const int MAX_PSK_TEXT_LEN  =  64;      
      const int WEP_KEY_SIZE      =  13;
      const int MAX_NUM_NETWORKS  =  16;
      const int MAX_SSID_SIZE     =  32;
      const int MAX_NAME_SIZE     =  20;
      const int MAX_PASSWORD_SIZE =  64;
      const int MAX_PROBE_AP      =  64;
      const int MAX_PATH          = 260;
      
      // roamMagicFactor range
      public const int ROAM_MAGIC_MIN =  1; 
      public const int ROAM_MAGIC_MAX = 10;  
      
      // Ad Hoc Channel range
      public const int ADHOCCHAN_MIN =  1;
      public const int ADHOCCHAN_MAX = 14;      
          
      
      // -----------------------------------------------------------------------------------------
      // API Errors
      // -----------------------------------------------------------------------------------------
      //#define WDQAPIERR int

      public const int ERR_SUCCESS    =   0;
      public const int ERR_NULLPTR    =  -1;  // A non-optional pointer argument was NULL
      public const int ERR_BADARG     =  -2;  // An argument value was invalid.
      public const int ERR_BADSIZE    =  -3;  // The size field of a passed-in structure was invalid.
      public const int ERR_GENERAL    =  -4;  // General operation-failed message. 
      public const int ERR_MEMORY     =  -5;  // Memory allocation failed.
      public const int ERR_BADWRITE   =  -6;  // A write operation failed.
      public const int ERR_BADREAD    =  -7;  // A read operation failed.
      public const int ERR_NOTFOUND   =  -8;  // An item was not found. 
      public const int ERR_BADOPEN    =  -9;  // A resource open operation error.
      public const int ERR_COLLISION  = -10;  // An item was found with the same name.
      public const int ERR_BADCREATE  = -11;  // A resource could not be created.
      public const int ERR_CFGLOCKED  = -12;  // A config change was attempted but config is locked.       
      
      
      const uint BITRATE_CCK_ONLY  = 0x0000000f;
      const uint BITRATE_OFDM_ONLY = 0x00000ff0;
      const uint BITRATE_MCS_ONLY  = 0xffff0000;
      
      const uint BITRATE_CCK_OFDM  = (BITRATE_CCK_ONLY | BITRATE_OFDM_ONLY);
      const uint BITRATE_ALL       = (BITRATE_MCS_ONLY | BITRATE_CCK_OFDM);
      
// -----------------------------------------------------------------------------------------
// Enumerated Types
// -----------------------------------------------------------------------------------------
//
// Notes:
// -- "NOVALUE" types can be used to indicate not-initialized or invalid value
// -- "BOUNDARY" types mark the end of the list
//

      // Power-save modes.
      public enum powerSaveMode_e {
	      NOVALUE = 0,
	      OFF , // Constantly Awake
	      FAST, // Normal                    
	      MAX , // Maximum
      } ;
      
      // Transmit power levels
      // NOTE: WDQAPI treats this as a numeric value in quarter-dBm units with -1 = max
      public enum txPowerLevel_e {
      	TXPWR_1   =  4,   //  1 dBM
      	TXPWR_5   = 20,   //  5 dBM
      	TXPWR_10  = 40,	  // 10 dBM
      	TXPWR_MAX = -1,   // Maximum power
      } ;

      // In API, this is a bit field, with b,g,a and n as distinct bits.  
      // Not all combinations are valid, however.  In WIM we're treating 
      // it as a list of discrete values.
      public enum bandSelect_e {
	     NOVALUE = 0,
         B   = 1<<0,         
	     G   = 1<<1,
         A   = 1<<2,	      
         N   = 1<<3,
         BG  = B|G ,
         ABG = A|BG,
      } ;

      // Controls whether or not the DFS chanels are scanned. 
      public enum dfsChanMode_e {
         NOVALUE = 0,      
         OFF,
         ON,
      } ;
      
      [Flags] // Bit mask of all b/g channels.
      public enum bgChannel_e {
         NOVALUE = 0,
         CH1  = 1<<0 ,
         CH2  = 1<<1 ,         
         CH3  = 1<<2 ,         
         CH4  = 1<<3 ,         
         CH5  = 1<<4 ,         
         CH6  = 1<<5 ,         
         CH7  = 1<<6 ,         
         CH8  = 1<<7 ,         
         CH9  = 1<<8 ,         
         CH10 = 1<<9 ,         
         CH11 = 1<<10,         
         CH12 = 1<<11,         
         CH13 = 1<<12,
         CH14 = 1<<13,
      } ;

      [Flags]  // Bit mask of all a/b/g bit rates.
      public enum abgRateMbps_e {
         NOVALUE = 0,
         _1   = 1<<0,         
         _2   = 1<<1,         
         _5_5 = 1<<2,         
         _11  = 1<<3,         
         _6   = 1<<4,         
         _9   = 1<<5,         
         _12  = 1<<6,         
         _18  = 1<<7,         
         _24  = 1<<8,         
         _36  = 1<<9,         
         _48  = 1<<10,         
         _54  = 1<<11,
      }
      
      [Flags]  // Misc. radio-related boolean settings.    
      public enum radioOption_e {   
         NOVALUE = 0,
         DISABLE_AUTO_CONNECT = 1<<0,
         REMOTE_CFG_ENABLE    = 1<<1,
         REMOTE_SRV_AUTO      = 1<<2,
      }   
      
      // Regulatory Domains
      public enum regDomain_e {
	      NOVALUE = 0,
	      AUTO ,
	      TELEC,
	      ETSI ,
	      FCC  ,
	      KOREA,
      } ;

      public enum antennaSelect_e {
	      NOVALUE = 0,
	      A0_ONLY,
	      A1_ONLY,
	      BOTH
      } ;

      [Flags]  // Network element option bits
      public enum netElemOpts_e {
	      NOVALUE      = 0,
	      ADHOC        = 1<<0,  // 1-AdHoc , 0-Infrastructure	
	      NOAUTOSWITCH = 1<<1,  // 1-Don't include in auto switch logic, 0-Do
      };

      // Security types
      public enum passwordType_e {
      	NOVALUE = 0,
      	NONE ,
      	WEP  ,
      	PSK  ,  // Personal WPA
      	WPA  ,  // Enterprise WPA
      	EAP  ,  // 802.1x
      } ;

      // 802.11 Authentication methods
      public enum authType80211_e {
      	NOVALUE = 0,
      	OPEN,
      	SHARED,
      	NETWORKEAP,
      } ; 
      
      // WPA version configuration (1, 2 or both)
      public enum wpaType_e {
      	NOVALUE = 0,
      	V1_ONLY,  // Only WPA1
      	V2_ONLY,  // Only WPA2
      	VER_ANY,  // WPA1 or WPA2
      } ;
      
      // EAP Methods.  (There can be more than one.)
      [Flags] public enum eapType_e {
		   NOVALUE     = 0,
		   LEAP        = 1<<0,
		   EAPFAST     = 1<<1,
		   PEAPGTC     = 1<<2,
		   PEAPMSCHAP  = 1<<3,
		   PEAPTLS     = 1<<4,
		   EAPTLS      = 1<<5,
      } ;

      // EAP Method Options.  (There can be more than one.)
      [Flags] public enum eapOpts_e {
         NOVALUE                 = 0,
         FORCE_SERVER_VALIDATION = 1<<0,
         ALLOW_ANON_PROVISIONING = 1<<1,
         ALLOW_PAC_PROVISIONING  = 1<<2,
      } ;
      
      // Authentication Hints  (There can be more than one.)
      [Flags] public enum authHints_e {
         NOVALUE          = 0,
         USE_SAVED_CREDS  = 1<<0,   // config contains saved login name and password
         USE_KEY_FILE     = 1<<1,   // config contains saved key file name and (maybe) a password   
         //NEW_LOGIN_PWD    = 1<<2,   // login password has been changed
         //NEW_KEYFILE_PWD  = 1<<3,   // key file password has been changed
         //NEW_WEP_KEYS     = 1<<4,   // wep keys have been changed
      } ;
      
      // Identifiers for specific radios or radio types      
      public enum radioType_e {
	      NOVALUE = 0,
	      UNKNOWN,
	      ABGN_BT,	      
      } ;

      // Pairwise Master Key Caching methods
      public enum pmkCacheType_e {
         NOVALUE = 0,
         PMK ,   // PMK Caching
         OKC ,   // Opportunistic PMK Caching
         CCKM,   // Cisco Centralized Key Management
      } ;

      // Connection Status
      public enum connStatus_e {
         NOVALUE = 0,
         NOT_CONNECTED,   
         ASSOCIATED   ,  // link-up with an AP
         CONNECTED    ,  // ready to go (authenticated, if applicable)
      } ;

      // Debug logging output options
      public enum debugType_e {
	      NOVALUE = 0,
	      OFF,
	      FILE,
	      RETAIL,
	      UDP,
      } ;
            
      // Special test modes
      public enum testModes_e{
	      NOVALUE = 0,
	      OFF,
	      RECEIVE,
	      TRANSMIT,
	      CARRIER,
      } ;
      
      // ------------------------------------------------------------------------------------------
      // Flags used to change the behavior of some API calls.  
      //
      // -- Note: Not all API's with a wdq_apiFlags_e argument will support all flags.  
      //          See each API function description for details.
      // ------------------------------------------------------------------------------------------
      [Flags] public enum apiFlags_e
      {
      	NONE = 0,
      	
      	// Used when setting configuration options, means fall back to existing settings if the 
      	// current connection is lost and cannot be restored. 	
      	ONLYIFCONNECT    = 1<<0,	
      	
      	// Used when setting path locations, means create the path if it does not exist
      	CREATEIFNOTEXIST = 1<<1,	
      	
         CHANGEPASSWORD   = 1<<2,

         // For config queries.  Means fill structures with default configuration settings.
         GETDEFAULTS      = 1<<3,

         // Got network element changes and queries, omit passwords and WEP keys.
         NOPRIVATEDATA    = 1<<4,               
         
      }  ;
      
      // ------------------------------------------------------------------------------------------
      // Flags that take the place of boolean values in scan list elements.
      // ------------------------------------------------------------------------------------------
      [Flags] 
      public enum scanElemOpts_e {
      	NONE = 0,
      	ADHOC       = 1<<0,	 // 0-Infrastructure, 1-AdHoc 	
      	CFG_EXISTS  = 1<<1,	 // 1-SSID exists in network store list (configured)
      }  ;

      // ------------------------------------------------------------------------------------------
      // Ping test status values.
      // ------------------------------------------------------------------------------------------
      public enum pingResultsStatus_e  {
         FINISHED  =  1,
         REPLYRECV =  0,
         TIMEOUT   = -1,
         ERROR     = -2, 
      }  ;      
      
// ------------------------------------------------------------------------------------------
//
// API Structures
//
// ------------------------------------------------------------------------------------------

//
// --- Special Fields ---
//

      // WEP key
      public struct WEPKEY_t 
      {
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = WEP_KEY_SIZE)] public byte[] _v;
         
         public override string ToString() { 
            return Encoding.ASCII.GetString(_v, 0, _v.Length); 
         }

         // Set the WEP key from a string.  (Invalid strings ignored.)          
         public static implicit operator WEPKEY_t(string s) {
            byte[] wep = new byte[WEP_KEY_SIZE];
            if (WEPKEY_t.IsValid(s)) {
               if (5 == s.Length || 13 == s.Length) { AscStrToBytes(s, ref wep); }
               else                                 { HexStrToBytes(s, ref wep); }
            }   
            return new WEPKEY_t { _v = wep };
         }
         
         // Test for a valid WEP key string:
         // - 5 or 13 ASCII digits (0x20 thru 0x7f)
         // - 10 or 26 hexadecimal digits  
         // - empty strings are valid too
         public static bool IsValid(string s, out int keylen) {
            string sRgxWepAny = @"^$|^[\x20-\x7f]{5}$|^[\x20-\x7f]{13}$|^[0-9a-fA-F]{10}$|^[0-9a-fA-F]{26}$";
            if (!Regex.Match(s, sRgxWepAny).Success) { keylen = 0; return false; }
            
            int len = s.Length;
            keylen = (10 == len || 26 == len) ? len/2 : len ;
            return true;
         }
         public static bool IsValidChar(char c) { 
            return (('\x20' <= c) && (c <= '\x7f')); 
         }   
            
         // convenience alias
         public static bool IsValid( string s ) { int n; return IsValid(s,out n); }
         
         // Set the WEP key from a byte array.   
         public static implicit operator WEPKEY_t(byte[] by) {
            byte[] wep = new byte[WEP_KEY_SIZE];
            Array.Copy(by, wep, Math.Min( by.Length, wep.Length));
            return new WEPKEY_t { _v = wep };            
         }
      }

      // Username: Fixed-length ASCII char buffer.  
      public struct USERNAME_t {
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_NAME_SIZE+1)]
         private byte[] _v;

         public static implicit operator string(USERNAME_t v) { return AscBufToStr(ref v._v); }
         public static implicit operator USERNAME_t(string s) { 
            return new USERNAME_t { _v = StrToAscBuf(s, Marshal.SizeOf(typeof(USERNAME_t))) };            
         }
         public override string ToString() { return AscBufToStr(ref _v); }
         public int Size() { return Marshal.SizeOf(this); }                  
      }  ;

      // Password: Fixed-length ASCII char buffer.  
      public struct PASSWORD_t {
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_PASSWORD_SIZE+1)]
         private byte[] _v;

         public static implicit operator byte[](PASSWORD_t v) { return v._v; }          
         public static implicit operator string(PASSWORD_t v) { return AscBufToStr(ref v._v); }
         public static implicit operator PASSWORD_t(string s) {
            return new PASSWORD_t { _v = StrToAscBuf(s, Marshal.SizeOf(typeof(PASSWORD_t))) };
         }
         public override string ToString() { return AscBufToStr(ref _v); }         
         public int Size() { return Marshal.SizeOf(this); }         
      }  ;


      // FYI: An example of Reflection:
      //   FieldInfo field = typeof(StructureToMarshalFrom).GetField("_value1");
      //   object[] attributes = field.GetCustomAttributes(typeof(MarshalAsAttribute), false);
      //   MarshalAsAttribute marshal = (MarshalAsAttribute)attributes[0];
      //   int sizeConst = marshal.SizeConst;
      public int SizeConst (System.Type typ) {
         var atr = typ.GetCustomAttributes(typeof(MarshalAsAttribute),false); 
         return ((MarshalAsAttribute)atr[0]).SizeConst; 
      }
      public int SizeConst(object var) {
         object[] attributes = var.GetType().GetCustomAttributes(typeof(MarshalAsAttribute), false);
         MarshalAsAttribute marshal = (MarshalAsAttribute)attributes[0];
         return marshal.SizeConst;
      }      
      
      // WPAPSK: 32-byte buffer
      //public struct WPAPSK_t {
      //   [MarshalAs(UnmanagedType.ByValArray, SizeConst = WPA_PSK_KEY_SIZE)] public byte[] _v;
      //   
      //   public void Construct()                         { _v = new byte[WPA_PSK_KEY_SIZE]; }
      //   public void Construct(string text, SSID_t ssid) { WdqGenWpaPsk(text, ref ssid, out this, 0); }
      //   
      //   public static bool IsValid( string text, ref SSID_t ssid  ) {
      //      return ERR_SUCCESS == WdqGenWpaPsk( text, ref ssid, (IntPtr)0, 0 );
      //   }
      //   public override int GetHashCode() { return _v.GetHashCode(); } // warning suppression         
      //}
      
      
      // This type is not used in any of the API data structures, only as an API argument type.
      // NOTE: WinCE does not support UnmanagedType.LPStr or CharSet.ANSI
      //public struct WPAPSKTEXT_t {
      //   [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_PSK_TEXT_LEN + 1)] private byte[] _v; 
      //   public WPAPSKTEXT_t( string s ) { this = s; }     
      //   public static implicit operator WPAPSKTEXT_t(string s) {
      //      if (MAX_PSK_TEXT_LEN < s.Length) throw new ArgumentException(String.Format("String too long:\"{0}\"","WPAPSKTEXT_t"));
      //      return new WPAPSKTEXT_t { _v = StrToAscBuf(s, MAX_PSK_TEXT_LEN + 1) };
      //   }
      //}            

      // SSID: integer (4-byte) length followed by a 32-byte buffer
      public struct SSID_t {
         public int    _len;       
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_SSID_SIZE)] public byte[] _v  ;

         public void Construct() {_len = 0; _v = new byte[MAX_SSID_SIZE]; }

         public static implicit operator string(SSID_t v) { 
            return AscBufToStr( ref v._v ).Substring(0,v._len); 
         }
         public static implicit operator SSID_t(string s) {
            return new SSID_t { _v   = StrToAscBuf( s, new SSID_t().Size() )
                              , _len = Math.Min( s.Length, new SSID_t().Size() ) } ;
         }
         public override string ToString() { return (string)this; } //AscBufToStr(ref _v).Substring(0,_len); }
         public int Length { get { return _len; } }
         public int Size() { return MAX_SSID_SIZE; }
         //public static int Size() { return Marshal.SizeOf(typeof(SSID_t)); }  // return Marshal.SizeOf(this);         

         // SSID is in the status structure so it needs a content-based comparison.         
         public override bool Equals(object obj) { return ArrayCompare<byte>(_v, ((SSID_t)obj)._v); }
         public override int  GetHashCode() { return _v.GetHashCode(); } // warning suppression
      }  ;  

      // Generic name string used for userIdentity, network name and client name
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]      
      public struct NAME_t {
         [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_NAME_SIZE+1)] private string _v;
         public static implicit operator string(NAME_t v) { return v._v; }
         public static implicit operator NAME_t(string s) { return new NAME_t { _v = s }; }
         //public static int Size() { return Marshal.SizeOf(typeof(NAME_t)) / Marshal.SizeOf(typeof(char)); }  // unicode string
         //public int Size() { return Marshal.SizeOf(_v) / Marshal.SizeOf(typeof(char)); }  // unicode string         
         public int Size() { return Marshal.SizeOf(new NAME_t()) / Marshal.SizeOf(typeof(char)); }  // unicode string                  
         public override string ToString() { return _v; }
      }  ;

      // Generic name string used for userIdentity, network name and client name
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct PATH_t {
         [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH+1)] private string _v;
         public static implicit operator string(PATH_t v) { return v._v; }
         public static implicit operator PATH_t(string s) { return new PATH_t { _v = s }; }
         //public static int Size() { return Marshal.SizeOf(typeof(PATH_t)) / Marshal.SizeOf(typeof(char)); }  // unicode string         
         public int Size() { return Marshal.SizeOf(new PATH_t()) / Marshal.SizeOf(typeof(char)); }  // unicode string
         public override string ToString() { return _v; }
      }  ;
      
      // Network list: fixed-length array of NAME_t, marshalled as a single, long char buffer
      [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)]
      public struct NETLIST_t 
      {
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = (MAX_NAME_SIZE+1) * MAX_NUM_NETWORKS)] 
         public char[] _list;            
      
         public string this[int i] {
            get {
               if (!IsValidIndex(i)) return "";
               var buf = new char[MAX_NAME_SIZE];
               Array.Copy(_list, i * (MAX_NAME_SIZE+1), buf, 0, MAX_NAME_SIZE );               
               return new string( buf ).TrimEnd(new char[]{'\0'}) ;
            }   
            set {
               if (IsValidIndex(i)) {
                  Array.Copy(value.ToCharArray(), 0, _list, i * (MAX_NAME_SIZE+1), Math.Min(value.Length, MAX_NAME_SIZE));
               } 
            }
         }
         
         public static bool IsValidIndex( int i ) {
            return (0<=i) && (i<MAX_NUM_NETWORKS) ;         
         }
         
         // This works on Windows, but on CE (CF 3.5) it throws System.NotSupportedException
         // -- CE does not seem to support marshalling arrays of user-defined types.
         // -- Note: Neither .NET 3.5 (Windows) or CF 3.5 (CE) seem to support marshalling
         //          of two-dimensional arrays.
         //[MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_NUM_NETWORKS)] public NETNAME_t[] networkList;      
      }

      // Version: UINT with special formatting
      [StructLayout(LayoutKind.Sequential)]
      public struct VERSION_t
      {
         private uint _v;
         
         public override string ToString() { 
            return String.Format("{0}.{1:00}.{2:0000}", _v >> 24, (_v >> 16) & 0xff, _v & 0xffff); 
         }
         public static implicit operator VERSION_t(uint u) {
            return new VERSION_t { _v = u };
         }
      }

      // MAC Address: 6-byte aray
      [StructLayout(LayoutKind.Sequential)]
      public struct MACADR_t {
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)] public byte[] _v;

         public void Construct() {_v = new byte[6]; }         
         
         public override string ToString() { 
            return BitConverter.ToString( _v ).Replace("-",":"); 
         }
         public override bool Equals(object obj) {
            return ArrayCompare<byte>( _v, ((MACADR_t)obj)._v );
         }
         // This is to address the following warning:
         // - warning CS0659: 'WdqApi.wdqapi.MACADR_t' overrides Object.Equals(object o) but does not override Object.GetHashCode()         
         //
         // NOTE: This hash may not produce the same value for two structures that our
         //       "Equals" override says are equal.
         public override int GetHashCode() {
            return _v.GetHashCode();
         }
      }

      // IP Address: UINT with special formatting.  (High byte is leftmost part of IP address)
      [StructLayout(LayoutKind.Sequential)]
      public struct IPADDR_t
      {
         private uint _v;
         
         public override string ToString() {
            return String.Format("{0}.{1}.{2}.{3}", _v >> 24, (_v >> 16) & 0xff, (_v >> 8) & 0xff, _v & 0xff);
         }
         public IPADDR_t(byte a, byte b, byte c, byte d) {
            _v = (uint)((a << 24) + (b << 16) + (c << 8) + d);
         }
         public static bool IsValid( string sip ) {
            //String sRgxIP = @"^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$" ;
            String sRgxByte = @"(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])"; // 0-255
            String sRgxIP = @"^" + sRgxByte + @"\." + sRgxByte + @"\." + sRgxByte + @"\." + sRgxByte + @"$" ;
            Match match = Regex.Match(sip.TrimEnd(null), sRgxIP );
            return match.Success;
         }
         public static implicit operator IPADDR_t(string s) {          
            if (!IsValid(s)) { 
               throw new System.ApplicationException(
                  String.Format( "IPADDR_t: Invalid IP string: '{0}'", s )
               );   
            }
            var ss = s.Split(new char[] { '.' });
            return new IPADDR_t( 
               byte.Parse(ss[0]),byte.Parse(ss[1]),byte.Parse(ss[2]),byte.Parse(ss[3])  
            );      
         }
         public static implicit operator IPADDR_t(uint u) { 
            return new IPADDR_t { _v = u }; 
         }
         public static implicit operator string(IPADDR_t ip) {          
            return ip.ToString();
         }
         public static implicit operator uint(IPADDR_t ip) {          
            return ip._v;
         }
         //public void Parse(out byte a, out byte b, out byte c, out byte d) {
         //   a = _v >> 24;
         //}
      } 
      
      // Boolean type.  We need this on CE because:
      // - UnmanagedType.Bool is apparently size=1 on Win CE
      // - CE does not support using UnmanagedType.U4 with managed type bool
      public struct BOOL_t
      {
         //[MarshalAs(UnmanagedType.U4)] private bool _v;      
         private uint _v;
         
         public override string ToString() { return (0 != _v).ToString(); }
         
         public static implicit operator bool(BOOL_t v) { return (0 == v._v) ? false : true ; }
         public static implicit operator BOOL_t(bool b) { return new BOOL_t{ _v = (uint)(b?1:0) }; }
      }

    
// ----------------------------------------------------------------------------
// Password Store Element 
// ----------------------------------------------------------------------------

      // WEP Keys
      [StructLayout(LayoutKind.Sequential)]
      public struct wepKeyStore_t {
         [MarshalAs(UnmanagedType.I4)]                                       public int txKey;
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = NUM_WEP_KEYS_MAX)] public int[] keylen;
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = WEP_KEY_SIZE * NUM_WEP_KEYS_MAX)] private byte[] _keys;

         public string this[int i]
         {
            get {
               if (!IsValidIndex(i)) throw new ArgumentOutOfRangeException( "Invalid WEP key index" );            
               var buf = GetKeyBytes(i);
               return Encoding.ASCII.GetString( buf, 0, buf.Length );
            }
            set {
               // Validate key and index
               // *T*O*D*O* -- What exceptions should we throw?  Define a custom WdqApiException?
               //              Probably we should use a custom one, derived from ApplicationException.
               int len;
               if (!WEPKEY_t.IsValid(value, out len)) throw new ArgumentException( "Invalid WEP key string", value );
               if (!IsValidIndex(i))                  throw new ArgumentOutOfRangeException( "Invalid WEP key index" );
            
               // Convert the string into a valid WEP key.
               WEPKEY_t wep = value;
            
               // Copy it into the byte array and set the length.
               Array.Copy( wep._v, 0, _keys, i*WEP_KEY_SIZE, WEP_KEY_SIZE );
               keylen[i] = len;
            }
         }

         public static bool IsValidIndex(int i) { 
            return (0 <= i) && (i < NUM_WEP_KEYS_MAX);
         }

         // The four WEP keys are stored in a single byte array.  Here we copy out the
         // portion of the array indicated by the index.
         public byte[] GetKeyBytes(int i) {
            if (!IsValidIndex(i)) return new byte[0];
            var buf = new byte[keylen[i]];
            Array.Copy(_keys, i * WEP_KEY_SIZE, buf, 0, buf.Length );
            return buf;
         }
         
         // This is what we would like to do, but on Win CE (CF 3.5) it throws System.NotSupportedException.
         // -- CE does not seem to support marshalling arrays of userdefined types.
         //[MarshalAs(UnmanagedType.ByValArray, SizeConst = NUM_WEP_KEYS_MAX)] public WEPKEY_t[] key;         
      } 
    
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct pwdStoreElement_t
      {
         public authType80211_e authType80211; 
         public passwordType_e  passwordType ;
         public wpaType_e       wpaVersion   ;         
         public eapType_e       eapMethods   ;
         public eapOpts_e       eapOptions   ;    
         public authHints_e     authHints    ;     
         public USERNAME_t      userIdentity ;
         [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH+1)] 
         public string          keyFile      ;
         public wepKeyStore_t   wepKeys      ;
         public PASSWORD_t      password     ;
         public PASSWORD_t      keyFilePwd   ;         
      }

// ----------------------------------------------------------------------------
//
// Network Store Element
//
// - one of these for each item in the top-level network list
// ----------------------------------------------------------------------------
     
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct networkStoreElem_t {
         public netElemOpts_e      options     ; 
         public int                order       ; // 0-based           
         public pmkCacheType_e     pmkCacheType; 
         public SSID_t             SSID        ; // (SSID_t combines ssid and ssidLen)
         public NAME_t             name        ; // matches name in network list
         public pwdStoreElement_t  pwdInfo     ; // password store element     
      }
            
// ----------------------------------------------------------------------------
// Network Store                                                               
// ----------------------------------------------------------------------------

      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct networkStore_t {
         public BOOL_t      autoSwitch;       
         public NAME_t      currentNetwork;
         public int         networkCount;
         public NETLIST_t   networkList;      
         public PATH_t      certStorePath;
      }  ;


// ----------------------------------------------------------------------------
// Radio configuration options
// ----------------------------------------------------------------------------

      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct radioStore_t {
         public NAME_t          clientName         ; // wdq_Char_t           clientName[WDQ_MAX_NAME_SIZE];   
         public radioOption_e   options            ; // wdq_radioOption_e    options;
         public txPowerLevel_e  txPowerQdbm        ; // int                  txPowerQdbm;        
         public powerSaveMode_e powerSaveMode      ; // wdq_powerSaveMode_e  powerSaveMode;
         public bandSelect_e    bands              ; // wdq_bandSelect_e	  bands;
         public bgChannel_e     bgChanMask         ; // unsigned int         bgChanMask;
         public abgRateMbps_e   bitRateMask        ; // unsigned int         bitRateMask;
         public dfsChanMode_e   dfsChanMode        ; // wdq_dfsChanMode_e    dfsChanMode;       
         public int             roamMagicFactor    ; // int                  roamMagicFactor;    
         public int             adhocChannel       ; // int                  adhocChannel;       
         public IPADDR_t        remoteServerIP     ; // wdq_IPaddr_t         remoteServerIP; 
         public testModes_e     testMode           ; // wdq_testModes_e      testMode; 
         public debugType_e     debugMode          ; // wdq_debugType_e      debugMode;   
         public antennaSelect_e antennaConfig      ; // wdq_antennaSelect_e  antennaConfig;     
         public int             nullPacketTimeS    ; // int                  nullPacketTimeS;    
         public int             disconnectScanTime ; // int                  disconnectScanTime; 
         public int             fragThresh         ; // int                  fragThresh;
         public int             rtsThresh          ; // int                  rtsThresh;   
      } ;

// ----------------------------------------------------------------------------
//    *** TBD ***
// ----------------------------------------------------------------------------
      [StructLayout(LayoutKind.Sequential)]
      public struct radioSpecificStore_t
      {
         public int dummy1;
      }   
  
// ----------------------------------------------------------------------------  
// All configuration data, including networks and radio options
// ----------------------------------------------------------------------------  
      [StructLayout(LayoutKind.Sequential)]
      public struct radioData_t
      {
         public networkStore_t networks;
         public radioStore_t radioStore;
         public radioSpecificStore_t radioSpecificData;
      } 
    
// ----------------------------------------------------------------------------  
// Status
// ----------------------------------------------------------------------------  
      
      [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
      public struct status_t {
         public VERSION_t    verDriver      ;
         public VERSION_t    verSupplicant  ;
         public radioType_e  adapterType    ;
         public regDomain_e  regDomain      ;         
         public MACADR_t     MAC            ;
         public MACADR_t     MACAP          ;
         public IPADDR_t     IPaddr         ;
         public IPADDR_t     IPaddrAP       ;
         public IPADDR_t     IPaddrRCServer ;
         public SSID_t       SSID           ;         
         public connStatus_e connStatus     ;
         public int          channel        ;           
         public int          RSSI           ;          
         public int          SNR            ;        
         public int          bitRate        ;
         public uint         framesTx       ;
         public uint         framesRx       ;
         public uint         bytesTx        ;        
         public uint         bytesRx        ;
         public BOOL_t       adapterReady   ;
         public BOOL_t       probing        ;                
      } ;

      //---------------------------------------------------------------------------------------
      // Ping Test Results
      //---------------------------------------------------------------------------------------

      [StructLayout(LayoutKind.Sequential)]
      public struct pingResults_t {
         public pingResultsStatus_e status;
         public int error;
         public int lost;
         public int recv;
         public int timeLast;
         public int timeTotal;
         public int timeMin;
         public int timeMax;
      } ;

// ----------------------------------------------------------------------------  
// Scan List
// ----------------------------------------------------------------------------  

      [StructLayout(LayoutKind.Sequential,Size=68)]
      public struct scanElement_t {
         public SSID_t         SSID       ;   
         public MACADR_t       MAC        ;         
         public passwordType_e netType    ;
         public scanElemOpts_e options    ;
         public bandSelect_e   band       ;
         public int            channel    ;   
         public int            RSSI       ; 
         public int            bitRateKHz ;   // maximum bit rate supported, in KHz
      }  ;
      
      // Need a constant for the elemBytes declaration.
      //const int scanElemSize = Marshal.SizeOf(typeof(scanElement_t));      
      const int scanElemSize = 68;  // sizeof(scanElement_t);

      [StructLayout(LayoutKind.Sequential)]
      public struct scanList_t {
         public int            elemCount;
         
         // The following works fine on Win32, but CE does not support arrays of user-defined structures.
         //[MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_PROBE_AP)] public scanElement_t[] elemList ;
         
         // Instead we treat it as an array of bytes, and provide a custom list abstraction.
         [MarshalAs(UnmanagedType.ByValArray, SizeConst = MAX_PROBE_AP * (scanElemSize + 4))] byte[] elemBytes;
         
         // The custom list abstraction, required only on CE because there's no native support for
         // marshalling arrays of user-define structs.
         public scanElement_t this[int i]
         {
            get {
               if (i < 0 || MAX_PROBE_AP < i) throw new ArgumentOutOfRangeException("Invalid scan element index");
               
               scanElement_t se = new scanElement_t ();
               se.SSID.Construct();
               se.MAC .Construct();
               
               int ofs = i * scanElemSize;
               
               se.SSID._len   =                 BitConverter.ToInt32( elemBytes, ofs );  ofs += Marshal.SizeOf( se.SSID._len );   
               Array.Copy( elemBytes, ofs, se.SSID._v, 0, se.SSID._v.Length ); ofs += se.SSID._v.Length;
               Array.Copy( elemBytes, ofs, se.MAC._v , 0, se.MAC ._v.Length ); ofs += se.MAC ._v.Length; ofs += 2; // padding
               se.netType     = (passwordType_e)BitConverter.ToUInt32( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );   
               se.options     = (scanElemOpts_e)BitConverter.ToUInt32( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );  
               se.band        = (bandSelect_e  )BitConverter.ToUInt32( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );  
               se.channel     =                 BitConverter.ToInt32 ( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );  
               se.RSSI        =                 BitConverter.ToInt32 ( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );  
               se.bitRateKHz  =                 BitConverter.ToInt32 ( elemBytes, ofs ); ofs += Marshal.SizeOf( typeof(int)  );  
               
               return se;
            }   
         }
      }  ;

//---------------------------------------------------------------------------------------
// API Functions
//---------------------------------------------------------------------------------------
      
      // WDQAPIERR DllExport WdqAddNetwork( wdq_netStoreElem_t *wdqNet, UINT reserved );      
      [DllImport("wdqapi")] public extern static int WdqAddNetwork( ref networkStoreElem_t elem, uint reserved );

      // WDQAPIERR DllExport WdqGetNetwork( const wdq_Char_t *name, wdq_netStoreElem_t *wdqNet, wdq_apiFlags_e flags );
      [DllImport("wdqapi")] public extern static int WdqGetNetwork([MarshalAs(UnmanagedType.LPWStr)] String name, out networkStoreElem_t elem, apiFlags_e flags);
      [DllImport("wdqapi")] public extern static int WdqGetNetwork(                                 IntPtr index, out networkStoreElem_t elem, apiFlags_e flags);

      //WDQAPIERR DllExport WdqChangeNetwork( const wdq_Char_t *name, wdq_netStoreElem_t *elem, wdq_apiFlags_e flags );
      [DllImport("wdqapi")] public extern static int WdqChangeNetwork( [MarshalAs(UnmanagedType.LPWStr)] String name, ref networkStoreElem_t elem, apiFlags_e flags );
      
      // WDQAPIERR DllExport WdqDeleteNetwork( const wdq_Char_t *name, wdq_apiFlags_e flags );
      [DllImport("wdqapi")] public extern static int WdqDeleteNetwork([MarshalAs(UnmanagedType.LPWStr)] String name, apiFlags_e flags);   
   
      // WDQAPIERR DllExport WdqEnableAutoNetworkSwitch( wdq_Bool_t enable, UINT reserved );
      [DllImport("wdqapi")] public extern static int WdqEnableAutoNetworkSwitch(bool enable, uint reserved);
   
      // WDQAPIERR DllExport WdqSetCurrentNetwork( const wdq_Char_t *name, wdq_apiFlags_e flags );
      [DllImport("wdqapi")] public extern static int WdqSetCurrentNetwork([MarshalAs(UnmanagedType.LPWStr)] string name, apiFlags_e flags);         
   
      // WDQAPIERR DllExport WdqSetCertPath( const wdq_Char_t *certPath, wdq_apiFlags_e flags );   
      [DllImport("wdqapi")] public extern static int WdqSetCertPath([MarshalAs(UnmanagedType.LPWStr)] String certStorePath, apiFlags_e flags);
      
      // WDQAPIERR DllExport WdqGetStatus( wdq_status_t *wdqStatus, UINT reserved);
      [DllImport("wdqapi")] public extern static int WdqGetStatus(out status_t data, uint reserved);                     
      
      // WDQAPIERR DllExport WdqGetConfiguration( wdq_radioData_t *wdqData, UINT reserved );
      [DllImport("wdqapi")] public extern static int WdqGetConfiguration(out radioData_t data, apiFlags_e flags);                     
   
      //UINT DllExport WdqGetVersion( UINT reserved );   
      [DllImport("wdqapi")] public extern static uint WdqGetVersion( uint reserved );      
   
      // WDQAPIERR DllExport WdqSetRadioParameters( const wdq_radioStore_t *wdqRadio, wdq_apiFlags_e flags );
      [DllImport("wdqapi")] public extern static int WdqSetRadioParameters(ref radioStore_t data, uint reserved);                  
   
      // WDQAPIERR DllExport WdqStartScan( UINT reserved );
      [DllImport("wdqapi")] public extern static int WdqStartScan();   

      //WDQAPIERR DllExport WdqGetScanList( wdq_scanList_t *pList, UINT reserved );      
      [DllImport("wdqapi")] public extern static int WdqGetScanList( out scanList_t list, uint reserved );         
      
      //typedef void (* wdq_fnDbgOut_t) ( const wdq_Char_t *psz );      
      public delegate void wdq_fnDbgOut_del( [MarshalAs(UnmanagedType.LPWStr)] string s );
      
      //void DllExport WdqSetDbgOut( const wdq_fnDbgOut_t pFunc, UINT reserved );
      [DllImport("wdqapi")] public extern static void WdqSetDbgOut( wdq_fnDbgOut_del callback, uint reserved );
      
      //typedef void (__stdcall * wdq_fnPingCallback_t) ( wdq_pingResults_t *pResults ); 
      public delegate void wdq_fnPingCallback_del( ref pingResults_t results, uint reserved );
            
      //WDQAPIERR DllExport WdqStartPingTest( const wdq_Char_t *ipAddr, int count, wdq_fnPingCallback_t pfn, UINT reserved ); 
      [DllImport("wdqapi")] public extern static int WdqStartPingTest([MarshalAs(UnmanagedType.LPWStr)] String ipAddr, int count, wdq_fnPingCallback_del callback, uint reserved);

      //WDQAPIERR DllExport WdqStopPingTest( UINT reserved );
      [DllImport("wdqapi")] public extern static int WdqStopPingTest( uint reserved );
      
      
      //WDQAPIERR DllExport WdqGenWpaPsk( const char *text, const wdq_SSID_t *ssid, wdq_Byte_t *key, UINT reserved ); 
      //NOTE: Win CE does not support UnmanagedType.LPStr     
      //[DllImport("wdqapi")] public extern static int WdqGenWpaPsk([MarshalAs(UnmanagedType.LPWStr)] String text, ref SSID_t ssid, out PASSWORD_t key, uint reserved);                  
      //[DllImport("wdqapi")] public extern static int WdqGenWpaPsk(ref WPAPSKTEXT_t text, ref SSID_t ssid, out PASSWORD_t key, uint reserved);            
      //[DllImport("wdqapi")] public extern static int WdqGenWpaPsk(ref WPAPSKTEXT_t text, ref SSID_t ssid,     IntPtr     key, uint reserved);      
      

//      [StructLayout(LayoutKind.Sequential)]
//      public struct WEPKEYS_t {
//         public WEPKEY_t key0; 
//         public WEPKEY_t key1; 
//         public WEPKEY_t key2; 
//         public WEPKEY_t key3; 
//              
//      }
      
      

//---------------------------------------------------------------------------------------            
// Utilities -- these really belong in a different file.
//---------------------------------------------------------------------------------------      
      
       // Convert a hexadecimal string to a byte array.
      // - make sure input string length is even
      // - will throw an exception if any digit is not valid hexadecimal
      // - DO NOT include a "0x" prefix 
      public static byte[] HexStrToBytes(string s, ref byte[] by ) {
         Array.Clear(by, 0, by.Length);
         for (int i = 0; i < s.Length; i += 2) {
            by[i/2] = Byte.Parse(s.Substring(i,2), NumberStyles.HexNumber); 
         }
         return by;
      }               
      public static byte[] HexStrToBytes(string s ) {      
         byte[] by = new byte[s.Length/2]; HexStrToBytes(s, ref by); return by;
      }
      
      
      // ----------------------------------------------------------------------------
      // String to Fixed-Length ASCII Byte Array Conversion
      //
      // Conventions:
      // -- string variables are never padded
      // -- byte arrays are padded with zeros when needed
      // -- if the string in a string-to-ASCII conversion is bigger than the byte
      //    arrays, the conversion is truncated to fit the buffer and there is no
      //    trailing zero.
      // ----------------------------------------------------------------------------      

      public static string AscBufToStr(ref byte[] by) {      
         return Encoding.ASCII.GetString(by, 0, by.Length).TrimEnd(new char[]{'\0'}); 
      }   
      
      public static void AscStrToBytes(string s, ref byte[] by) {
         Array.Clear(by, 0, by.Length);
         Array.Copy( Encoding.ASCII.GetBytes(s), by, Math.Min(s.Length,by.Length));
      }
      
      public static byte[] StrToAscBuf(string s, int size) { return Encoding.ASCII.GetBytes(s.PadRight(size, '\0')); }
      
      public static byte[] StrToAscBuf(string s          ) { return Encoding.ASCII.GetBytes(s);                      }

      // Compare two arrays, element by element.
      public static bool ArrayCompare<T>(T[] a1, T[] a2) {
         if ((a1 == null) && (a2 == null)) { return true; }
         if ((a1 == null) || (a2 == null)) { return false; }
         if (a1.Length != a2.Length) { return false; }
         for (int i = 0; i < a1.Length; i++) { if (!a1[i].Equals(a2[i])) { return false; } }
         return true;
      }
   }
}

/*

// ============================================================================
//
// wdqapi.h 
//
//    "C" header file that declares the public members of the WDQAPI DLL
//
// NOTE: This file is intended to be cross-platform compatible.
//
// ============================================================================


#ifndef _WDQAPI_H_
#define _WDQAPI_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

// Ensure that _WIN32 is defined in your project for Windows CE support 
#ifdef _WIN32
  #if defined(WDQAPI_STATIC_LIB)
    #define DllExport WINAPI
  #elif defined(WDQAPIDLL_EXPORTS)
    #define DllExport __declspec(dllexport) WINAPI
  #else
    #define DllExport __declspec(dllimport) WINAPI
  #endif
#else
  #define DllExport 
#endif


// *** AP scan list element has been updated ***



// ----------------------------------------------------------------------------------------
//
// Ping Test: setup and results
// ----------------------------------------------------------------------------------------

// Defines the parameters of a ping test.
typedef struct {
   wdq_IPaddr_t addr   ; // target address (0.0.0.0 = default target) 
   UINT         count  ; // number of echo requests to send (0 = continuous)
   UINT         size   ; // size in bytes of echo request payload
   UINT         timeout; // max time in milliseconds to wait for an echo response
   UINT         delay  ; // min time in milliseconds between echo requests   
} wdq_pingConfig_t;

// Used to report cumulative results of a ping test.
typedef struct {
   UINT         sent   ; // number of echo requests sent
   UINT         recv   ; // number of echo responses received  (# lost = send-recv)
   UINT         minTime; // time in milliseconds of shortest response time
   UINT         maxTime; // time in milliseconds of longest response time  
   UINT         avgTime; // time in milliseconds of average response time  
} wdq_pingResults_t;


// ----------------------------------------------------------------------------  
//
// API Function prototypes 
//
// ----------------------------------------------------------------------------  


// ----------------------------------------------------------------------------------------
// Returns the version number of the API.  
//   Format is Major.miNor.Revision with the digits assigned MMNNRRRR
//   as in the following examples: 1010010 = 1.01.0010, 12345678 = 12.34.5678
// ----------------------------------------------------------------------------------------
UINT DllExport WdqGetVersion( UINT reserved );

// Macros for parsing out the pieces of the version number
#define WDQ_VER_MAJ(v)  ((v)/1000000)
#define WDQ_VER_MIN(v) (((v)/10000)%100)
#define WDQ_VER_REV(v)  ((v)%10000)

// ----------------------------------------------------------------------------------------
//
// Pass-through to allow direct access to driver OID calls
//
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetOid(unsigned int oid, void *data, unsigned int *dataSize, UINT reserved);
WDQAPIERR DllExport WdqSetOid(unsigned int oid, void *data, unsigned int  dataSize, UINT reserved);

// ----------------------------------------------------------------------------------------
// Starts a ping test using the settings in the passed ping config structure.  Clears any
// previous ping results.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqStartPingTest( wdq_pingConfig_t *p ); 

// ----------------------------------------------------------------------------------------
// Stops a ping test if one is in progress. 
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqStopPingTest( );

// ----------------------------------------------------------------------------------------
// Returns cumulative results of most recent ping test.  
// -- May be called repeatedly while a test is in progress, as well as after a test has
//    stopped.  Always succeeds: if no test has been run returns zeroed results.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqGetPingResults( wdq_pingResults_t *p );

// ----------------------------------------------------------------------------------------
// Unlocks the configuration data, which allows changes to be made.  
// - returns WDQ_APIERR_BADARG, if password is incorrect.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqUnlockConfig( wdq_Char_t *password, UINT reserved );

// ----------------------------------------------------------------------------------------
// Locks the configuration data.  
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqLockConfig( );

// ----------------------------------------------------------------------------------------
// Changes the password used to unlock the configuration data.
// - returns WDQ_APIERR_BADARG, if oldPassword is incorrect.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqChangeAdminPassword( wdq_Char_t *oldPassword, wdq_Char_t *newPassword, UINT reserved );

// ----------------------------------------------------------------------------------------
// Enables and disables connection activity logging.
// ----------------------------------------------------------------------------------------
WDQAPIERR DllExport WdqEnableLogging( wdq_Bool_t bEnable, wdq_debugType_e dbgType, UINT reserved );




#ifdef __cplusplus
}
#endif

#endif  // _WDQAPI_H_

*/
