/*******************************************************************************
 * Project:  InterfaceServer
 * @file     ModuleSwitch.cpp
 * @brief 
 * @author   lbh
 * @date:    2016年7月6日
 * @note
 * Modify history:
 ******************************************************************************/
#include <fstream>
#include "ModuleSwitch.hpp"

namespace inter
{

ModuleSwitch::ModuleSwitch(const std::string& strModulePath)
    : neb::Module(strModulePath), pStepSwitch(nullptr)
{
}

ModuleSwitch::~ModuleSwitch()
{
    for (auto iter = m_mapModuleConf.begin();
             iter != m_mapModuleConf.end(); ++iter)
    {
        if (iter->second != NULL)
        {
            delete iter->second;
            iter->second = NULL;
        }
    }
    m_mapModuleConf.clear();
}

bool ModuleSwitch::Init()
{
    std::string strConfFile = GetWorkPath() + std::string("/conf/ModuleSwitch.json");
    LOG4_DEBUG("CONF FILE = %s.", strConfFile.c_str());

    std::ifstream fin(strConfFile.c_str());
    if (fin.good())
    {
        std::stringstream ssContent;
        neb::CJsonObject oSwitchConf;
        ssContent << fin.rdbuf();
        fin.close();
        if (oSwitchConf.Parse(ssContent.str()))
        {
            std::string strUrlPath;
            int iCmd = 0;
            neb::CJsonObject* pModuleJson = NULL;
            if (oSwitchConf["module"].IsEmpty())
            {
                LOG4_WARNING("oSwitchConf[\"module\"] is empty!");
            }
            for (int i = 0; i < oSwitchConf["module"].GetArraySize(); ++i)
            {
                if (std::string("/im/hello_switch") == oSwitchConf["module"][i]("url_path"))
                {
                    continue;
                }
                bool bAcceptModule = false;
                pModuleJson = new neb::CJsonObject(oSwitchConf["module"][i]);
                if (pModuleJson->Get("url_path", strUrlPath) && pModuleJson->Get("cmd", iCmd))
                {
                    for (int j = 0; j < (*pModuleJson)["node_type"].GetArraySize(); ++j)
                    {
                        if ((*pModuleJson)["node_type"](j) == GetNodeType())
                        {
                            m_mapModuleConf.insert(std::make_pair(strUrlPath, pModuleJson));
                            bAcceptModule = true;
                            break;
                        }
                    }
                    if (!bAcceptModule)
                    {
                        DELETE(pModuleJson);
                    }
                }
                else
                {
                    LOG4_ERROR("miss \"url_paht\" or \"cmd\" in oSwitchConf[\"module\"][%d]", i);
                    DELETE(pModuleJson);
                }
            }
        }
        else
        {
            LOG4_ERROR("oSwitchConf pasre error");
            return(false);
        }
    }
    else
    {
        //配置信息流读取失败
        LOG4_ERROR("Open conf \"%s\" error!", strConfFile.c_str());
        return(false);
    }

    return(true);
}

bool ModuleSwitch::AnyMessage(
                std::shared_ptr<neb::SocketChannel> pUpstreamChannel,
                const HttpMsg& oInHttpMsg)
{
    if (HTTP_OPTIONS == oInHttpMsg.method())
    {
        LOG4_TRACE("receive an OPTIONS");
        ResponseOptions(pUpstreamChannel, oInHttpMsg);
        return(true);
    }
    auto module_conf_iter = m_mapModuleConf.find(oInHttpMsg.path());
    if (module_conf_iter == m_mapModuleConf.end())
    {
        HttpMsg oOutHttpMsg;
        LOG4_ERROR("no cmd config for %s in \"ModuleSwitch.json\"!", oInHttpMsg.path().c_str());
        oOutHttpMsg.set_type(HTTP_RESPONSE);
        oOutHttpMsg.set_status_code(404);
        oOutHttpMsg.set_http_major(oInHttpMsg.http_major());
        oOutHttpMsg.set_http_minor(oInHttpMsg.http_minor());
        SendTo(pUpstreamChannel, oOutHttpMsg);
        return(false);
    }

    pStepSwitch = std::dynamic_pointer_cast<StepSwitch>(MakeSharedStep("inter::StepSwitch", pUpstreamChannel, oInHttpMsg, module_conf_iter->second));
    if ((pStepSwitch))
    {
        if (neb::CMD_STATUS_RUNNING == pStepSwitch->Emit(neb::ERR_OK))
        {
            LOG4_TRACE("pStepSwitch running");
            return(true);
        }
    }
    return(false);
}

void ModuleSwitch::Response(std::shared_ptr<neb::SocketChannel> pUpstreamChannel, const HttpMsg& oInHttpMsg, int iErrno, const std::string& strErrMsg)
{
    HttpMsg oHttpMsg;
    neb::CJsonObject oResponseData;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(oInHttpMsg.http_major());
    oHttpMsg.set_http_minor(oInHttpMsg.http_minor());
    oResponseData.Add("code", iErrno);
    oResponseData.Add("msg", strErrMsg);
    if (iErrno == 0)
    {
        oResponseData.Add("data", neb::CJsonObject("[]"));
    }
    oHttpMsg.set_body(oResponseData.ToFormattedString());
    SendTo(pUpstreamChannel, oHttpMsg);
}

void ModuleSwitch::ResponseOptions(std::shared_ptr<neb::SocketChannel> pUpstreamChannel, const HttpMsg& oInHttpMsg)
{
    LOG4_DEBUG("%s()", __FUNCTION__);
    HttpMsg oHttpMsg;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(oInHttpMsg.http_major());
    oHttpMsg.set_http_minor(oInHttpMsg.http_minor());
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Connection", "keep-alive"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Origin", "*"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Headers", "Origin, Content-Type, Cookie, Accept"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Methods", "GET, POST"));
    oHttpMsg.mutable_headers()->insert(google::protobuf::MapPair<std::string, std::string>("Access-Control-Allow-Credentials", "true"));
    SendTo(pUpstreamChannel, oHttpMsg);
}

} /* namespace inter */
