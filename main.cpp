#include <podofo/podofo.h>
#include <iostream>
#include <stdexcept>


using namespace PoDoFo;
using namespace std;

void removeJavaScript(PdfMemDocument& document) {
    // Check if there are any document actions, print them out and remove them
    PdfDictionary& catalog = document.GetCatalog().GetDictionary();

    // Check and remove any openActions
    if (catalog.HasKey(PdfName("OpenAction"))) {
        catalog.RemoveKey(PdfName("OpenAction"));
        cout << "Document OpenAction Removed" << endl;
    }

    // loop through the acroform and remove any actions
    PdfAcroForm* acroform = document.GetAcroForm();
    if (acroform) {
        const unsigned int numFields = acroform->GetFieldCount();
        for (int i=0; i< numFields; i++) {
            PdfField& field = acroform->GetFieldAt(i);
            PdfDictionary& dict = field.GetDictionary();

            // Check if the field has any actions
            if (dict.HasKey(PdfName("A"))) {
                dict.RemoveKey(PdfName("A"));
            }

            // check if the field has any additional actions
            if (dict.HasKey(PdfName("AA"))) {
                dict.RemoveKey(PdfName("AA"));
            }
        }
    }
}

void clearMetadata(PdfMemDocument& document, const string& filename) {
    std::vector<string> emptyKeywords;
    PdfMetadata& info = document.GetMetadata();
    info.SetAuthor(PdfString(""));
    info.SetCreator(PdfString(""));
    info.SetKeywords(emptyKeywords);
    info.SetProducer(PdfString(""));
    info.SetSubject(PdfString(""));

    // set the title to the filename
    size_t pos = filename.find_last_of(("/\\"));
    string title = (pos == string::npos) ? filename : filename.substr(pos + 1);
    info.SetTitle(PdfString(title));
}

void updateAcroform(PdfMemDocument& document) {
    // Method to update the Default Appearance of the fields in the PDF Acroform Field Dictionary
    PdfString update_DA = "/Helv 0 Tf 0 0 1 rg";
    PdfString update_V = "";

    // check if the acroform exists
    PdfAcroForm* acroform = document.GetAcroForm();
    if(!acroform) {
        cerr << "No AcroForm found in this document." << endl;
        return;
    }


    // See if any fields exist in the document
    PdfObject* fields = acroform->GetDictionary().GetKey(PdfName("Fields"));
    if (!fields || !fields->IsArray()) {
        cerr << "No Fields found in this document" << endl;
        return;
    }


    // Drill down into fields
    const unsigned int numFields = acroform->GetFieldCount();
    for (int i=0; i< numFields; i++) {
        PdfField& field = acroform->GetFieldAt(i);

        cout << field.GetDictionary().ToString() << endl;


        // TEXT BOX EDITS
        if (field.GetType() == PdfFieldType::TextBox) {
            PdfDictionary& dict = field.GetDictionary();

            // Check what the default appearance is and if it exists
            // If so, replace it with the new Default Appearance
            PdfObject* default_DA = dict.GetKey(PdfName("DA"));
            if (default_DA) {
                default_DA->SetString(update_DA);
            }

            // Check if there is any Text in the TextBox
            // Update it to be blank if it does exist
            PdfObject* default_V = dict.GetKey((PdfName("V")));
            if (default_V) {
                default_V->SetString(update_V);
            }

            // Check if the textbox has set border color/fill color
            // Remove it if it exists
            PdfObject* default_MK = dict.GetKey((PdfName("MK")));
            if (default_MK) {
                dict.RemoveKey(PdfName("MK"));
            }

            // Remove default appearance if it exists
            PdfObject* default_AP = dict.GetKey(PdfName("AP"));
            if (default_AP) {
                // update the ap to reflect the new appearance
                dict.RemoveKey(PdfName("AP"));
            }
        }
        else {
            // CHECKBOX EDITS
            if (field.GetType() == PdfFieldType::CheckBox) {
                // Get the dictionary of the field
                PdfDictionary& dict = field.GetDictionary();

                // check if the field is set to yes
                PdfObject* on = dict.GetKey(PdfName("AS"));
                if (on && on->IsName() && on->GetName().GetString() != "Off") {
                    dict.AddKey(PdfName("AS"), PdfName("Off"));
                }

                if (!on) {
                    // If the button does not have a state, check if it has kids
                    PdfObject* kids = dict.GetKey(PdfName("Kids"));
                    if (!kids || !kids->IsArray()) {
                        cerr << "No Kids found in this document" << endl;
                        return;
                    }

                    // Get the kids array
                    // If only 1 kid, replace parent with kid
                    PdfArray& kidsArray = kids->GetArray();
                    if (kidsArray.size() == 1) {
                        PdfObject* pKidObj = document.GetObjects().GetObject(kidsArray[0].GetReference());
                        if (pKidObj && pKidObj->IsDictionary()) {
                            PdfDictionary& childDictionary = pKidObj->GetDictionary();
                            PdfObject* child_activated = childDictionary.GetKey(PdfName("AS"));
                            if (child_activated) {
                                childDictionary.AddKey(PdfName("AS"), PdfName("Off"));
                            }
                            PdfObject* child_V = childDictionary.GetKey(PdfName("V"));
                            if (child_V) {
                                child_V->SetString(update_V);
                            }
                            PdfObject* child_MK = childDictionary.GetKey(PdfName("MK"));
                            if (child_MK) {
                                PdfObject* default_CA = child_MK->GetDictionary().GetKey(PdfName("CA"));
                                // Set BC
                                PdfArray borderColor;
                                borderColor.Add(PdfVariant(0.0));
                                child_MK->GetDictionary().AddKey(PdfName("BC"), borderColor);

                                // Set BG
                                PdfArray fillColor;
                                fillColor.Add(PdfVariant(1.0));
                                child_MK->GetDictionary().AddKey(PdfName("BG"), fillColor);

                                if (default_CA) {
                                    // Remove the CA key
                                    child_MK->GetDictionary().RemoveKey(PdfName("CA"));
                                }

                            }
                        }
                    }
                }

                // Check if the field contains a V
                if (dict.HasKey(PdfName("V"))) {
                    dict.RemoveKey(PdfName("V"));
                }

                // Set the DA to blue
                PdfObject* default_DA = dict.GetKey(PdfName("DA"));
                if (default_DA) {
                    default_DA->SetString("/Helv 0 Tf 0 0 1 rg");
                }

                // Remove MK key (inidcates if it should be specially filled
                PdfObject* default_MK = dict.GetKey(PdfName("MK"));
                if (default_MK) {
                    PdfObject* default_CA = default_MK->GetDictionary().GetKey(PdfName("CA"));
                        // Set BC
                        PdfArray borderColor;
                        borderColor.Add(PdfVariant(0.0));
                        default_MK->GetDictionary().AddKey(PdfName("BC"), borderColor);

                        // Set BG
                        PdfArray fillColor;
                        fillColor.Add(PdfVariant(1.0));
                        default_MK->GetDictionary().AddKey(PdfName("BG"), fillColor);

                    if (default_CA) {
                        // Remove the CA key
                        default_MK->GetDictionary().RemoveKey(PdfName("CA"));
                    }
                }
                // Work with the appearance settings
                PdfObject* default_AP = dict.GetKey(PdfName("AP"));
                 if (default_AP) {
                     // Get Normal appearance (N)
                     PdfObject* default_N = default_AP->GetDictionary().GetKey(PdfName("N"));
                     if (default_N) {
                         // Two options for the checkbox, on and off
                         PdfObject* default_Off = default_N->GetDictionary().GetKey(PdfName("Off"));
                         PdfObject* default_On = default_N->GetDictionary().GetKey(PdfName("Yes"));

                         if (default_Off) {
                             // Get the object and the stream
                             PdfObject* default_off_OBJ = document.GetObjects().GetObject(default_Off->GetReference());
                             auto* stream = dynamic_cast<PdfObjectStream*>(default_off_OBJ->GetStream());

                             ifstream file("checkBox_AP_off.txt", std::ios::binary | std::ios::ate);
                             streamsize size = file.tellg();
                             file.seekg(0, std::ios::beg);

                             std::vector<char> buffer(size);
                             if (!file.read(buffer.data(), size)) {
                                 throw runtime_error("Cannot read stream");
                             }

                             bufferview bufferview1(buffer.data(), buffer.size());
                                stream->SetData(bufferview1, false);

                         }

                         if (default_On) {
                             // Get the object and the stream
                             PdfObject* default_on_OBJ = document.GetObjects().GetObject(default_On->GetReference());
                             auto* stream = dynamic_cast<PdfObjectStream*>(default_on_OBJ->GetStream());

                             ifstream file("checkBox_AP_on.txt", std::ios::binary | std::ios::ate);
                             streamsize size = file.tellg();
                             file.seekg(0, std::ios::beg);

                             std::vector<char> buffer(size);
                             if (!file.read(buffer.data(), size)) {
                                 throw runtime_error("Cannot read stream");
                             }

                             bufferview bufferview1(buffer.data(), buffer.size());
                             stream->SetData(bufferview1, false);
                         }
                     }

                     // Get the pressed appearance (D)
                     PdfObject* default_D = default_AP->GetDictionary().GetKey(PdfName("D"));
                     if (default_D) {

                         // Get the two states
                         PdfObject* default_Off = default_D->GetDictionary().GetKey(PdfName("Off"));
                         PdfObject* default_On = default_N->GetDictionary().GetKey(PdfName("Yes"));

                         if (default_Off) {
                             // Get object and stream
                             PdfObject* default_off_OBJ = document.GetObjects().GetObject(default_Off->GetReference());
                             auto* stream = dynamic_cast<PdfObjectStream*>(default_off_OBJ->GetStream());

                             // Get the saved stream and read into a string buffer
                             ifstream file("checkBox_AP_off_D.txt", std::ios::binary | std::ios::ate);
                             streamsize size = file.tellg();
                             file.seekg(0, std::ios::beg);

                             std::vector<char> buffer(size);
                             if (!file.read(buffer.data(), size)) {
                                 throw runtime_error("Cannot read stream");
                             }

                             bufferview bufferview1(buffer.data(), buffer.size());
                             stream->SetData(bufferview1, false);
                         }
                         if (default_On) {
                             // Get objeect and stream
                             PdfObject* default_on_OBJ = document.GetObjects().GetObject(default_On->GetReference());
                             auto* stream = dynamic_cast<PdfObjectStream*>(default_on_OBJ->GetStream());

                             // Get the saved stream and read into a string buffer
                             ifstream file("checkBox_AP_on_D.txt", std::ios::binary | std::ios::ate);
                             streamsize size = file.tellg();
                             file.seekg(0, std::ios::beg);

                             std::vector<char> buffer(size);
                             if (!file.read(buffer.data(), size)) {
                                 throw runtime_error("Cannot read stream");
                             }

                             bufferview bufferview1(buffer.data(), buffer.size());
                             stream->SetData(bufferview1, false);

                         }

                     }
                 }
            }
            // RADIOBUTTON EDITS
            if (field.GetType() == PdfFieldType::RadioButton) {

                // Get the dictionary of the field
                PdfDictionary& dict = field.GetDictionary();

                // Check if the field contains a DA
                PdfObject* default_DA = dict.GetKey(PdfName("DA"));
                if (default_DA) {
                    default_DA->SetString("/Helv 0 Tf 0 0 1 rg");
                };

                // check if the field contains a V
                if (dict.HasKey(PdfName("V"))) {
                    dict.RemoveKey(PdfName("V"));
                }

                // Each radio button field has two children objects (yes or no)
                // We need to alter both the DA of the full button and the DA of the children
                PdfObject* kids = dict.GetKey(PdfName("Kids"));
                if (!kids || !kids->IsArray()) {
                    cerr << "No Kids found in this document" << endl;
                    return;
                }

                PdfArray& kidsArray = kids->GetArray();
                for (const auto & j : kidsArray) {
                    if (j.IsReference()) {
                        PdfObject* pKidObj = document.GetObjects().GetObject(j.GetReference());
                        if (pKidObj && pKidObj->IsDictionary()) {

                            // Change the DA
                            PdfObject* default_DAKid = pKidObj->GetDictionary().GetKey(PdfName("DA"));
                            if (default_DAKid) {
                                default_DAKid->SetString("/Zadb 0 Tf 0 0 1 rg");
                            }

                            // Set AS to Off
                            PdfObject* on = pKidObj->GetDictionary().GetKey(PdfName("AS"));
                            if (on && on->IsName() && on->GetName().GetString() != "Off") {
                                pKidObj->GetDictionary().AddKey(PdfName("AS"), PdfName("Off"));
                            }

                            // if BS remove BS
                            // This is border style
                            PdfObject* default_BS = pKidObj->GetDictionary().GetKey(PdfName("BS"));
                            if (default_BS) {
                                pKidObj->GetDictionary().RemoveKey(PdfName("BS"));
                            }

                            // Set MK , BC and BG
                            PdfObject* default_MK = pKidObj->GetDictionary().GetKey(PdfName("MK"));
                            if (default_MK) {
                                PdfObject* default_CA = default_MK->GetDictionary().GetKey(PdfName("CA"));
                                if (default_CA) {
                                    // Set BC
                                    PdfArray borderColor;
                                    borderColor.Add(PdfVariant(0.0));
                                    default_MK->GetDictionary().AddKey(PdfName("BC"), borderColor);

                                    // Set BG
                                    PdfArray fillColor;
                                    fillColor.Add(PdfVariant(1.0));
                                    default_MK->GetDictionary().AddKey(PdfName("BG"), fillColor);

                                    // Remove the CA key
                                    default_MK->GetDictionary().RemoveKey(PdfName("CA"));
                                }
                            }

                            // Get the appearance Stream
                            PdfObject* default_AP = pKidObj->GetDictionary().GetKey(PdfName("AP"));
                            if (default_AP) {

                                // Get the normal appearance
                                PdfObject* default_N = default_AP->GetDictionary().GetKey(PdfName("N"));
                                if (default_N) {

                                    // Get the off and on normal appearance
                                    PdfObject* default_Off = default_N->GetDictionary().GetKey(PdfName("Off"));
                                    PdfObject* default_On = default_N->GetDictionary().GetKey(PdfName("Yes"));
                                    PdfObject* default_No= default_N->GetDictionary().GetKey(PdfName("No"));

                                    // if they exists and are references get the object
                                    if (default_Off && default_Off->IsReference()) {
                                        // IMPLEMENT
                                        // Get the object and the stream
                                        PdfObject* default_off_OBJ = document.GetObjects().GetObject(default_Off->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_off_OBJ->GetStream());

                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_off.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);

                                    }
                                    if (default_On && default_On->IsReference()) {
                                        // Get the obect
                                        PdfObject* default_on_OBJ = document.GetObjects().GetObject(default_On->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_on_OBJ->GetStream());


                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_yes.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);

                                    }
                                    if (default_No && default_No->IsReference()) {
                                        // Get the obect
                                        PdfObject* default_no_OBJ = document.GetObjects().GetObject(default_No->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_no_OBJ->GetStream());

                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_no.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);
                                    }
                                }

                                // Do the same thing for D
                                PdfObject* default_D = default_AP->GetDictionary().GetKey(PdfName("D"));
                                if (default_D) {
                                    // Get the pressed appearances
                                    PdfObject* default_Off = default_D->GetDictionary().GetKey(PdfName("Off"));
                                    PdfObject* default_On = default_D->GetDictionary().GetKey(PdfName("Yes"));
                                    PdfObject* default_No= default_D->GetDictionary().GetKey(PdfName("No"));

                                    // if they exists and are references get the object
                                    if (default_Off && default_Off->IsReference()) {
                                        // IMPLEMENT
                                        // Get the object and the stream
                                        PdfObject* default_off_OBJ = document.GetObjects().GetObject(default_Off->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_off_OBJ->GetStream());

                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_off.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);

                                    }
                                    if (default_On && default_On->IsReference()) {
                                        // Get the obect
                                        PdfObject* default_on_OBJ = document.GetObjects().GetObject(default_On->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_on_OBJ->GetStream());


                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_yes.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);

                                    }
                                    if (default_No && default_No->IsReference()) {
                                        // Get the obect
                                        PdfObject* default_no_OBJ = document.GetObjects().GetObject(default_No->GetReference());
                                        auto* stream = dynamic_cast<PdfObjectStream*>(default_no_OBJ->GetStream());

                                        // Get the saved stream and read into a string buffer
                                        ifstream file("radioButton_AP_no.txt", std::ios::binary | std::ios::ate);
                                        streamsize size = file.tellg();
                                        file.seekg(0, std::ios::beg);

                                        std::vector<char> buffer(size);
                                        if (!file.read(buffer.data(), size)) {
                                            throw runtime_error("Cannot read stream");
                                        }

                                        bufferview bufferview1(buffer.data(), buffer.size());
                                        stream->SetData(bufferview1, false);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) { // Check if there are exactly two arguments (argv[1] and argv[2])
        std::cerr << "Usage: " << argv[0] << " <input file> <output file>" << std::endl;
        return 1;
    }

    const char* inputFileName = argv[1];
    const char* outputFileName = argv[2];

    try{
        PdfMemDocument doc;
        doc.Load(inputFileName);
        updateAcroform(doc);
        removeJavaScript(doc);
        clearMetadata(doc, inputFileName);

        // Save the document in a subfolder called normalized
        string outfile = "normalized/" + string(outputFileName);
        doc.Save(outfile);


    }catch (const PdfError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}