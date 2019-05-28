/*******************************************************************************
 * Project:  NebulaInterface
 * @file     ModuleHello.cpp
 * @brief 
 * @author   Bwar
 * @date:    2019年1月27日
 * @note
 * Modify history:
 ******************************************************************************/
#include <fstream>
#include "ModuleHello.hpp"

namespace inter
{

ModuleHello::ModuleHello(const std::string& strModulePath)
    : neb::Module(strModulePath)
{
}

ModuleHello::~ModuleHello()
{
}

bool ModuleHello::Init()
{
    return(true);
}

bool ModuleHello::AnyMessage(
                std::shared_ptr<neb::SocketChannel> pUpstreamChannel,
                const HttpMsg& oInHttpMsg)
{
    HttpMsg oHttpMsg;
    neb::CJsonObject oResponseData;
    oHttpMsg.set_type(HTTP_RESPONSE);
    oHttpMsg.set_status_code(200);
    oHttpMsg.set_http_major(oInHttpMsg.http_major());
    oHttpMsg.set_http_minor(oInHttpMsg.http_minor());
    oResponseData.Add("code", 0);
    oResponseData.Add("msg", "hello!");
    oHttpMsg.set_body(oResponseData.ToFormattedString());
    SendTo(pUpstreamChannel, oHttpMsg);
    return(true);
}

} /* namespace inter */
