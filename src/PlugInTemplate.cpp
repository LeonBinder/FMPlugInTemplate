// <copyright company="CAScination AG">
//     This file contains proprietary code. Copyright (c) 2024. All rights reserved
// </copyright>

// Libraries =================================================
#include "../Headers/FMWrapper/FMXTypes.h"
#include "../Headers/FMWrapper/FMXText.h"
#include "../Headers/FMWrapper/FMXFixPt.h"
#include "../Headers/FMWrapper/FMXData.h"
#include "../Headers/FMWrapper/FMXCalcEngine.h"

#include <sstream>
#include <string>

// FileMaker Constants ==========================
//Plugin ID
static const char* kFMtp("FMtp");

// Function ID, Min- & Max-Args
enum { kFMtp_FunctionTemplateID = 100, kFMtp_FunctionTemplateMin = 3, kFMtp_FunctionTemplateMax = 3 };

//Function Name, Definition & Description for FileMaker
static const char* kFMtp_FunctionTemplateName("FMtp_FunctionTemplate");
static const char* kFMtp_FunctionTemplateDefinition("FMtp_FunctionTemplate( param1 ; param2 ; param3 )");
static const char* kFMtp_FunctionTemplateDescription("Takes Parameters from FileMaker and writes back ...");

// External Function
std::tuple<std::string, std::string, std::string> ExternalFunction(std::string param1, std::string param2, std::string param3)
{
    std::string res1, res2, res3;

    return std::make_tuple(res1, res2, res3);
}

// Helper Functions ============================================
// Convert fmx::Text to std::string
std::string FMTextToString(const fmx::Text& fmText) {
    fmx::uint32 size = fmText.GetSize();
    std::vector<char> buffer(size * 4 + 1);  // UTF-8 characters can be up to 4 bytes.
    fmText.GetBytes(buffer.data(), size * 4, fmx::Text::kEncoding_UTF8);
    return std::string(buffer.data());
}

// Escape JSON special characters
std::string escape_json(const std::string& text) 
{
    // Create an output string stream to store the escaped string
    std::ostringstream buf;

    // Iterate over each character in the input string
    for (auto c = text.cbegin(); c != text.cend(); c++) 
    {
        // Check the current character and perform the appropriate escape sequence
        switch (*c) 
        {
            case '"': 
                // Escape double quotes by adding a backslash
                buf << "\\\""; 
                break;
            case '\\': 
                // Escape backslashes by adding another backslash
                buf << "\\\\"; 
                break;
            case '\b': 
                // Escape backspace character
                buf << "\\b"; 
                break;
            case '\f': 
                // Escape form feed character
                buf << "\\f"; 
                break;
            case '\n': 
                // Escape newline character
                buf << "\\n"; 
                break;
            case '\r': 
                // Escape carriage return character
                buf << "\\r"; 
                break;
            case '\t': 
                // Escape tab character
                buf << "\\t"; 
                break;
            default:
                // If the character doesn't need escaping, add it directly to the output
                buf << *c;
        }
    }

    return buf.str();
}

// Create JSON from values
std::string create_json(const std::string& var1, const std::string& var2, const std::string& var3)
{
    std::ostringstream json;

    json << "{";
    json << "\"Variable1\":\"" << escape_json(var1) << "\",";
    json << "\"Variable2\":\"" << escape_json(var2) << "\",";
    json << "\"Variable3\":\"" << escape_json(var3) << "\"";
    json << "}";

    return json.str();
}

// Write Values to FileMaker =============================
static FMX_PROC(fmx::errcode) Do_FMtp_FunctionTemplate(short /* funcId */, const fmx::ExprEnv& environment, const fmx::DataVect& dataVect, fmx::Data& results)
{
    fmx::errcode errorResult(958);
    fmx::TextUniquePtr outText;
    const fmx::Locale* outLocale(&results.GetLocale());

    if (dataVect.Size() == 3)
    {
        // Retrieve and Convert FileMaker Data to string
        std::string param1 = FMTextToString(dataVect.AtAsText(0));
        std::string param2 = FMTextToString(dataVect.AtAsText(1));
        std::string param3 = FMTextToString(dataVect.AtAsText(2));

        // Generate EEPROM data
        auto result_tuple = ExternalFunction(param1, param2, param3);

        // Create JSON result
        std::string jsonResult = create_json(std::get<0>(result_tuple), std::get<1>(result_tuple), std::get<2>(result_tuple));

        // Set as Result to FileMaker
        outText->Assign(jsonResult.c_str(), fmx::Text::kEncoding_UTF8);
        errorResult = 0;
        results.SetAsText(*outText, *outLocale);
    }

    return errorResult;
}

// Plugin Initialization ==========================================
static fmx::ptrtype Do_PluginInit(fmx::int16 version)
{
    fmx::ptrtype result(static_cast<fmx::ptrtype>(kDoNotEnable));
    const fmx::QuadCharUniquePtr pluginID(kFMtp[0], kFMtp[1], kFMtp[2], kFMtp[3]);
    fmx::TextUniquePtr name;
    fmx::TextUniquePtr definition;
    fmx::TextUniquePtr description;
    fmx::uint32 flags(fmx::ExprEnv::kDisplayInAllDialogs | fmx::ExprEnv::kFutureCompatible);

    if (version >= k150ExtnVersion)
    {
        name->Assign(kFMtp_FunctionTemplateName, fmx::Text::kEncoding_UTF8);
        definition->Assign(kFMtp_FunctionTemplateDefinition, fmx::Text::kEncoding_UTF8);
        description->Assign(kFMtp_FunctionTemplateDescription, fmx::Text::kEncoding_UTF8);
        if (fmx::ExprEnv::RegisterExternalFunctionEx(*pluginID, kFMtp_FunctionTemplateID, *name, *definition, *description, kFMtp_FunctionTemplateMin, kFMtp_FunctionTemplateMax, flags, Do_FMtp_FunctionTemplate) == 0)
        {
            result = kCurrentExtnVersion;
        }
    }

    return result;
}

// Plugin Shutdown ===============================================
static void Do_PluginShutdown(fmx::int16 version)
{
    const fmx::QuadCharUniquePtr pluginID(kFMtp[0], kFMtp[1], kFMtp[2], kFMtp[3]);

    if (version >= k150ExtnVersion)
    {
        static_cast<void>(fmx::ExprEnv::UnRegisterExternalFunction(*pluginID, kFMtp_FunctionTemplateID));
    }
}

// Copy UTF-8 string to unichar16 string ==========================
static void CopyUTF8StrToUnichar16Str(const char* inStr, fmx::uint32 outStrSize, fmx::unichar16* outStr)
{
    fmx::TextUniquePtr txt;
    txt->Assign(inStr, fmx::Text::kEncoding_UTF8);
    const fmx::uint32 txtSize((outStrSize <= txt->GetSize()) ? (outStrSize - 1) : txt->GetSize());
    txt->GetUnicode(outStr, 0, txtSize);
    outStr[txtSize] = 0;
}

// Get plugin strings ============================================
static void Do_GetString(fmx::uint32 whichString, fmx::uint32 /* winLangID */, fmx::uint32 outBufferSize, fmx::unichar16* outBuffer)
{
    switch (whichString)
    {
    case kFMXT_NameStr:
        CopyUTF8StrToUnichar16Str("GenEEPROMPlugIn", outBufferSize, outBuffer);
        break;

    case kFMXT_AppConfigStr:
        CopyUTF8StrToUnichar16Str("FileMaker PlugIn to generate EEPROM String, Checksum and Specification", outBufferSize, outBuffer);
        break;

    case kFMXT_OptionsStr:
        // Characters 1-4 are the plug-in ID
        CopyUTF8StrToUnichar16Str(kFMtp, outBufferSize, outBuffer);

        // Character 5 is always "1"
        outBuffer[4] = '1';

        // Character 6
        // Use "Y" if you want to enable the Configure button for plug-ins in the Preferences dialog box.
        // Use "n" if there is no plug-in configuration needed.
        // If the flag is set to "Y" then make sure to handle the kFMXT_DoAppPreferences message.
        outBuffer[5] = 'n';

        // Character 7 is always "n"
        outBuffer[6] = 'n';

        // Character 8
        // Set to "Y" if you want to receive kFMXT_Init/kFMXT_Shutdown messages.
        // In most cases, you want to set it to 'Y' since it's the best time to register/unregister your plugin functions.
        outBuffer[7] = 'Y';

        // Character 9
        // Set to "Y" if the kFMXT_Idle message is required.
        // For simple external functions this may not be needed and can be turned off by setting the character to "n".
        outBuffer[8] = 'n';

        // Character 10
        // Set to "Y" to receive kFMXT_SessionShutdown and kFMXT_FileShutdown messages.
        outBuffer[9] = 'n';

        // Character 11 is always "n"
        outBuffer[10] = 'n';

        // NULL terminator
        outBuffer[11] = 0;
        break;

    case kFMXT_HelpURLStr:
        CopyUTF8StrToUnichar16Str("http://httpbin.org/get?id=", outBufferSize, outBuffer);
        break;

    default:
        outBuffer[0] = 0;
        break;
    }
}

// FileMaker External Call Procedure =============================
FMX_ExternCallPtr gFMX_ExternCallPtr(nullptr);

void FMX_ENTRYPT FMExternCallProc(FMX_ExternCallPtr pb)
{
    // Setup global defined in FMXExtern.h (this will be obsoleted in a later header file)
    gFMX_ExternCallPtr = pb;

    // Message dispatcher
    switch (pb->whichCall)
    {
    case kFMXT_Init:
        pb->result = Do_PluginInit(pb->extnVersion);
        break;

    case kFMXT_Idle:
        break;

    case kFMXT_Shutdown:
        Do_PluginShutdown(pb->extnVersion);
        break;

    case kFMXT_DoAppPreferences:
        break;

    case kFMXT_GetString:
        Do_GetString(static_cast<fmx::uint32>(pb->parm1), static_cast<fmx::uint32>(pb->parm2), static_cast<fmx::uint32>(pb->parm3), reinterpret_cast<fmx::unichar16*>(pb->result));
        break;

    case kFMXT_SessionShutdown:
        break;

    case kFMXT_FileShutdown:
        break;

    } // switch whichCall

} // FMExternCallProc