/*******************************************************************************
 * Project:  NebulaInterface
 * @file     ModuleHello.hpp
 * @brief    Http请求示例
 * @author   Bwar
 * @date:    2019年1月27日
 * @note     
 * Modify history:
 ******************************************************************************/
#ifndef MODULEHELLO_HPP_
#define MODULEHELLO_HPP_

#include <string>
#include <util/json/CJsonObject.hpp>
#include <actor/cmd/Module.hpp>

namespace inter
{

class ModuleHello: public neb::Module, public neb::DynamicCreator<ModuleHello, std::string>
{
public:
    ModuleHello(const std::string& strModulePath);
    virtual ~ModuleHello();

    virtual bool Init();

    virtual bool AnyMessage(
                    std::shared_ptr<neb::SocketChannel> pUpstreamChannel,
                    const HttpMsg& oHttpMsg);
protected:
    void Response(std::shared_ptr<neb::SocketChannel> pUpstreamChannel,
                    const HttpMsg& oInHttpMsg,
                    int iErrno, const std::string& strErrMsg);
};

} /* namespace inter */

#endif /* MODULEHELLO_HPP_ */
