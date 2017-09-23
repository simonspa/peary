/*
 * ATLASPix_Config.cpp
 *
 *  Created on: 22 Sep 2017
 *      Author: peary
 */




#include "ATLASPix_Config.h"

ATLASPix_Config::ATLASPix_Config()
{

}

bool ATLASPix_Config::AddParameter(std::string name, unsigned int bits, int shiftdirection, unsigned int initial)
{
    if(bits == 0 || bits > 30)
        return false;

    //set initial value to maximum, if it is larger than the parameter
    if(initial >= ((unsigned int)1 << bits))
        initial = (1 << bits) - 1;

    indextoname.insert(std::make_pair(parameters.size(), name));
    nametoindex.insert(std::make_pair(name, parameters.size()));

    std::stringstream bitorder("");
    switch(shiftdirection)
    {
    case(MSBFirst):
        bitorder << (bits-1);
        for(int i = (int)bits-2; i >= 0; --i)
            bitorder << "," << i;
        break;
    case(LSBFirst):
        bitorder << 0;
        for(int i = 1; i < (int)bits; ++i)
            bitorder << "," << i;
        break;
    default:
        return false;
    }

    parameters.push_back(std::make_pair(bitorder.str(), initial));

    return true;
}

bool ATLASPix_Config::AddParameter(std::string name, std::string bitorder, unsigned int initial)
{
    //the parameter is eihter empty or too large for this class:
    if(bitorder.length() < 1 || bitorder.length() > 80)
        return false;

    //check the bitorder string for errors:
    for(unsigned int i = 0; i < bitorder.size(); ++i)
    {
        if(bitorder.c_str()[i] > 57 || (bitorder.c_str()[i] < 48 && bitorder.c_str()[i] != 44))
            return false;
    }

    //check for completeness:
    int bitvector = 0;
    int bitvalue;
    unsigned long int pos = 0;
    unsigned long int oldpos = 0;
    while(pos != std::string::npos)
    {
        pos = bitorder.find(',', oldpos);
        std::stringstream s("");
        s << bitorder.substr(oldpos, pos - oldpos);
        s >> bitvalue;

        bitvector |= 1 << bitvalue;
        oldpos = pos + 1;
    }

    //check for missing bits:
    for(int i=1; i < 30; ++i)
    {
        if(bitvector < (1 << i) - 1)
            return false;
        else if(bitvector == (1 << i) - 1)
            break;
    }

    indextoname.insert(std::make_pair(parameters.size(), name));
    nametoindex.insert(std::make_pair(name, parameters.size()));

    parameters.push_back(std::make_pair(bitorder, initial));

    return true;
}

void ATLASPix_Config::ClearParameters()
{
    parameters.clear();
    nametoindex.clear();
    indextoname.clear();
}

int ATLASPix_Config::GetParameterIndexToName(std::string name)
{
    auto index = nametoindex.find(name);

    if(index != nametoindex.end())
        return index->second;
    else
        return -1;
}

std::string ATLASPix_Config::GetParameterNameToIndex(unsigned int index)
{
    auto name = indextoname.find(index);

    if(name != indextoname.end())
        return name->second;
    else
        return "";
}

int ATLASPix_Config::GetParameter(unsigned int index)
{
    if(index >= parameters.size())  //return -1 for invalid indices
        return -1;
    else
        return parameters[index].second;
}

int ATLASPix_Config::GetParameter(std::string name)
{
    auto index = nametoindex.find(name);
    //check for validity of the returned index:
    if(index != nametoindex.end())
        return parameters[index->second].second;
    else    //return -1 for invalid names
        return -1;
}

int ATLASPix_Config::GetParameterWidth(unsigned int index)
{
    if(index >= parameters.size())
        return -1;

    int maxbit = 0;
    int thisbit;
    unsigned long int oldpos = 0;
    unsigned long int pos    = 0;
    //std::stringstream s("");
    while(pos != std::string::npos)
    {
        pos = parameters[index].first.find(',', oldpos);
        std::stringstream s("");
        s.str(parameters[index].first.substr(oldpos, pos-oldpos));
        //std::cout << s.str() << std::endl;
        s >> thisbit;

        if(thisbit > maxbit)
            maxbit = thisbit;

        oldpos = pos + 1;
    }

    return maxbit + 1;
}

int ATLASPix_Config::GetParameterWidth(std::string name)
{
    auto index = nametoindex.find(name);
    //check for validity of the returned index:
    if(index == nametoindex.end())
        return -1;

    return GetParameterWidth(index->second);
}

std::string ATLASPix_Config::GetParameterBitOrder(unsigned int index)
{
    if(index >= parameters.size())
        return "";
    else
        return parameters[index].first;
}

std::string ATLASPix_Config::GetParameterBitOrder(std::string name)
{
    auto index = nametoindex.find(name);
    if(index == nametoindex.end())
        return "";
    else
        return parameters[index->second].first;
}

std::string ATLASPix_Config::GetParameterName(unsigned int index)
{
    auto name = indextoname.find(index);

    if(name != indextoname.end())
        return name->second;
    else
        return "";
}

bool ATLASPix_Config::SetParameter(unsigned int index, unsigned int value)
{
    if(index >= parameters.size())
    {
        std::cout << "Index out of range!" << std::endl;
        return false;
    }


    unsigned int size = 1 << GetParameterWidth(index);

    if(value >= size)
    {
        std::cout << "Value out of range!" << std::endl;
        return false;
    }

    parameters[index].second = value;
    return true;
}

bool ATLASPix_Config::SetParameter(std::string name, unsigned int value)
{
    auto index = nametoindex.find(name);

    if(index == nametoindex.end())
    {
        std::cout << "Name \"" << name << "\" was not found!" << std::endl;
        return false;
    }

    return SetParameter(index->second, value);
}

std::vector<bool> ATLASPix_Config::GenerateBitVector(int shiftdirection)
{
    //return an empty vector on invalid shiftdirection
    if(shiftdirection < 0 && shiftdirection > 3)
        return std::vector<bool>();

    bool globalinvert = (shiftdirection == GlobalInvertedLSBFirst || shiftdirection == GlobalInvertedMSBFirst);
    if(shiftdirection == GlobalInvertedLSBFirst)
        shiftdirection = LSBFirst;
    else if (shiftdirection == GlobalInvertedMSBFirst)
        shiftdirection = MSBFirst;

    std::vector<bool> bitvector;

    //int index = 0;
    for(auto it =parameters.begin(); it !=parameters.end(); ++it)
    {
        //shows what was put into the bitstream:
        //std::cout << indextoname.find(index)->second << " " << it->first << std::endl;
        //++index;  //needed for the name output of the contents of the bitstream

        //send LSB first on reversed parameter XOR reversed shift direction:
        if(shiftdirection == MSBFirst)
        {
            unsigned long int pos = 0;
            unsigned long int oldpos = 0;
            int value = 0;
            while(pos != std::string::npos)
            {
                std::stringstream s("");
                pos = it->first.find(',', oldpos);
                s << it->first.substr(oldpos, pos - oldpos);
                s >> value;
                oldpos = pos + 1;
                bitvector.push_back((it->second & (1 << value))?true:false);
            }
        }
        //normally, the MSB is sent first on sending MSBFirst
        else
        {
            unsigned long int pos = it->first.length();
            unsigned long int oldpos = std::string::npos;
            int value = 0;
            while(pos != std::string::npos)
            {
                std::stringstream s("");
                pos = it->first.rfind(',', oldpos);
                s << it->first.substr(pos + 1, oldpos - pos);
                s >> value;
                oldpos = pos - 1;
                bitvector.push_back((it->second & (1 << value))?true:false);
            }
        }
    }

    if (globalinvert)
    {
    std::vector<bool> temp;
    temp.insert(temp.end(),bitvector.rbegin(),bitvector.rend());
    return temp;
    }

    return bitvector;
}

/*
tinyxml2::XMLError ATLASPix_Config::LoadFromXMLFile(std::string filename)
{
    parameters.clear();

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
    if(error != tinyxml2::XML_NO_ERROR)
        return error;

    tinyxml2::XMLNode* config = getNode(&doc, "ShiftRegister");
    tinyxml2::XMLElement* child = config->FirstChildElement();

    while(child != 0)
    {
        //old format (before switching to Richard compatible format):
        //int bits = child->IntAttribute("Bits");
        //int writedirection = MSBFirst;
        //if(std::string(child->Attribute("WriteDirection")).compare("LSBFirst") == 0)
        //    writedirection = LSBFirst;
        //int value = child->IntAttribute("Value");

        if(std::string(child->Value()).compare("Register") == 0)
        {
            int bits = 0;
            std::string writedirection;
            int value;
            if(child->QueryIntAttribute("value", &value) != tinyxml2::XML_NO_ERROR)
                value = 0;

            tinyxml2::XMLElement* grandchild = child->FirstChildElement();
            while(grandchild != 0)
            {
                if(std::string(grandchild->Name()).compare("Attribute") == 0)
                {
                    std::string name = std::string(grandchild->Attribute("name"));
                    if(name.compare("sr.size") == 0)
                    {
                        if(grandchild->QueryIntText(&bits) != tinyxml2::XML_NO_ERROR)
                            bits = 1;
                    }
                    else if(name.compare("sr.WriteDirection") == 0)
                    {
                        if(std::string(grandchild->GetText()).compare("LSBFirst") == 0)
                            writedirection = "LSBFirst";
                        else if(std::string(grandchild->GetText()).compare("MSBFirst") == 0)
                            writedirection = "MSBFirst";
                        else
                            writedirection = std::string(grandchild->GetText());
                    }
                    //redundant, not needed as value is a pararmeter of `child`, not a child of `child`:
                    else if(name.compare("sr.Value") == 0)
                    {
                        if(grandchild->QueryIntText(&value) != tinyxml2::XML_NO_ERROR)
                            value = 0;
                    }
                }

                if(grandchild != child->LastChildElement())
                    grandchild = grandchild->NextSiblingElement();
                else
                    grandchild = 0;
            }

            //std::cout << child->Attribute("name") << "\t\t" << bits << " " << value << std::endl;

            //add the parameter with the MSB/LSB notation:
            if(bits != 0)
            {
                if(writedirection.compare("MSBFirst") == 0)
                    AddParameter(std::string(child->Attribute("name")), bits, MSBFirst, value);
                else if(writedirection.compare("LSBFirst") == 0)
                    AddParameter(std::string(child->Attribute("name")), bits, LSBFirst, value);
            }
            //add the parameter with the bitorder notation:
            else
                AddParameter(std::string(child->Attribute("name")), writedirection, value);
        }


        if(child != config->LastChildElement())
            child = child->NextSiblingElement();
        else
            child = 0;
    }

    return error;
}

tinyxml2::XMLError ATLASPix_Config::SaveToXMLFile(std::string filename, std::string devicename)
{
    //create a new XML Document:
    tinyxml2::XMLDocument doc;

    //include the XML declaration:
    tinyxml2::XMLDeclaration* dec = doc.NewDeclaration("xml version=\"1.0\"");
    doc.LinkEndChild(dec);

    tinyxml2::XMLElement* node;

    //Add save date to the file:
    QDateTime date = QDateTime::currentDateTime();

    //add the configuration:
    node = doc.NewElement("ShiftRegister");
    node->SetAttribute("name", devicename.c_str());
    node->SetAttribute("SaveDate", date.toString().toStdString().c_str());

    tinyxml2::XMLElement* configpar;

    for(unsigned int i = 0; i < parameters.size(); ++i)
    {
        configpar = doc.NewElement("Register");
        configpar->SetAttribute("name", indextoname.find(i)->second.c_str());
        configpar->SetAttribute("value",parameters[i].second);

        tinyxml2::XMLElement* attribute;
        attribute = doc.NewElement("Attribute");
        attribute->SetAttribute("name","sr.size");
        attribute->SetText(parameters[i].first.c_str());
        configpar->LinkEndChild(attribute);

        attribute = doc.NewElement("Attribute");
        attribute->SetAttribute("name", "sr.WriteDirection");
        attribute->SetText(parameters[i].first.c_str());
        configpar->LinkEndChild(attribute);

        //old format (before switching to Richard compatible format):
        //configpar->SetAttribute("Bits", abs(parameters[i].first));
        //configpar->SetAttribute("WriteDirection", ((parameters[i].first < 0)?"LSBFirst":"MSBFirst"));
        //configpar->SetAttribute("Value", parameters[i].second);

        node->LinkEndChild(configpar);
    }

    doc.LinkEndChild(node);

    tinyxml2::XMLError error = doc.SaveFile(filename.c_str());

    return error;
}*/
