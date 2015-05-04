// =============================================================================
//
// Copyright (c) 2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#pragma once


#include <istream>
#include "Poco/UUID.h"
#include "Poco/Net/MediaType.h"
#include "Poco/Net/PartHandler.h"

#include "ofLog.h"

#include "ofx/HTTP/BaseRoute.h"
#include "ofx/HTTP/PostRouteEvents.h"


namespace ofx {
namespace HTTP {


/// \brief Settings for a PostRoute.
class PostRouteSettings: public BaseRouteSettings
{
public:
    /// \brief Create PostRouteSettings.
    /// \param routePathPattern The regex pattern that this route will handle.
    /// \param requireSecurePort True if this route requires
    ///        communication on an SSL encrypted port.
    PostRouteSettings(const std::string& routePathPattern = DEFAULT_POST_ROUTE,
                      bool requireSecurePort = false);

    /// \brief Destroy the PostRouteSetting.
    virtual ~PostRouteSettings();

    void setUploadFolder(const std::string& uploadFolder);
    const std::string& getUploadFolder() const;

    void setUploadRedirect(const std::string& uploadRedirect);
    const std::string& getUploadRedirect() const;

    void setWriteBufferSize(std::size_t writeBufferSize);
    std::size_t getWriteBufferSize() const;

    void setFieldLimit(std::size_t fieldLimit);
    std::size_t getFieldLimit() const;

    /// \brief Set the maximum file upload size in bytes.
    /// \param maximumFileUploadSize The maximum file upload size in bytes.
    void setMaximumFileUploadSize(unsigned long long maximumFileUploadSize);

    /// \brief Get the maximum file upload size.
    /// \returns The maximum file upload size in bytes.
    unsigned long long getMaximumFileUploadSize() const;

    static const std::string DEFAULT_POST_ROUTE;
    static const std::string DEFAULT_POST_FOLDER;
    static const std::string DEFAULT_POST_REDIRECT;

    /// \brief Default values.
    enum Defaults
    {
        /// \brief File upload buffer.
        DEFAULT_POST_BUFFER_SIZE = 8192,
        /// \brief Maximum number of form fields.
        DEFAULT_FIELD_LIMIT = 100,
        /// \brief Maximum file upload size (2 MB)
        DEFAULT_MAXIMUM_FILE_UPLOAD_SIZE = 2097152
    };

    /// \brief An unfortunate compromise until C++11.
    /// \note C++ is not able to initialize static collections until
    ///        after C++11.  This is a compromise until then.
    static const std::string DEFAULT_POST_HTTP_METHODS_ARRAY[];

    /// \brief The default HTTP methods for this route.
    static const HTTPMethodSet DEFAULT_POST_HTTP_METHODS;
    
private:
    std::string _uploadFolder;
    std::string _uploadRedirect;
    std::size_t _writeBufferSize;
    std::size_t _fieldLimit;
    unsigned long long _maximumFileUploadSize;
    
};


/// \brief A route for handing HTTP POST requests. 
class PostRoute: public BaseRoute_<PostRouteSettings>
{
public:
    /// \brief A typedef for the PostRouteSettings.
    typedef PostRouteSettings Settings;

    PostRoute(const Settings& settings = Settings());
    
    virtual ~PostRoute();

    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

    PostRouteEvents& getEvents();

    PostRouteEvents events;

    template<class ListenerClass>
    void registerPostEvents(ListenerClass* listener);

    template<class ListenerClass>
    void unregisterPostEvents(ListenerClass* listener);

};


template<class ListenerClass>
void PostRoute::registerPostEvents(ListenerClass* listener)
{
    ofAddListener(events.onHTTPPostEvent, listener, &ListenerClass::onHTTPPostEvent);
    ofAddListener(events.onHTTPFormEvent, listener, &ListenerClass::onHTTPFormEvent);
    ofAddListener(events.onHTTPUploadEvent, listener, &ListenerClass::onHTTPUploadEvent);
}


template<class ListenerClass>
void PostRoute::unregisterPostEvents(ListenerClass* listener)
{
    ofRemoveListener(events.onHTTPPostEvent, listener, &ListenerClass::onHTTPPostEvent);
    ofRemoveListener(events.onHTTPFormEvent, listener, &ListenerClass::onHTTPFormEvent);
    ofRemoveListener(events.onHTTPUploadEvent, listener, &ListenerClass::onHTTPUploadEvent);
}


/// \brief A flexible POST route handler.
///
/// Form data must be encoded with "multipart/form-data" or
/// "application/x-www-form-urlencoded".  "text/plain" form
/// encoding is supported, but not parsed.
class PostRouteHandler: public AbstractRouteHandler
{
public:
    /// \brief A typedef for PostRouteSettings
    typedef PostRouteSettings Settings;

    /// \brief Create a PostRouteHandler.
    /// \param parent The parent PostRoute.
    PostRouteHandler(PostRoute& parent);

    /// \brief Destroy the PostRouteHandler.
    virtual ~PostRouteHandler();

    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response);

    void stop();

    /// \brief A constant defining "text/plain".
    static const Poco::Net::MediaType POST_CONTENT_TYPE_TEXT_PLAIN;

    /// \brief A constant defining "multipart/form-data".
    static const Poco::Net::MediaType POST_CONTENT_TYPE_MULTIPART;
    
    /// \brief A constant defining "application/x-www-form-urlencoded".
    static const Poco::Net::MediaType POST_CONTENT_TYPE_URLENCODED;
    
    /// \brief A constant defining "application/json".
    static const Poco::Net::MediaType POST_CONTENT_TYPE_JSON;
    
    
private:
    /// \brief The parent PostRoute reference.
    PostRoute& _parent;
    
};


class PostRouteFileHandler: public Poco::Net::PartHandler
{
public:
    PostRouteFileHandler(PostRoute& parent,
                         const std::string& sessionId,
                         const Poco::Net::HTTPServerRequest& request,
                         const std::string& formId);

    virtual ~PostRouteFileHandler();

    void handlePart(const Poco::Net::MessageHeader& header,
                    std::istream& stream);

    bool isContentTypeValid(const std::string& contentType) const;

private:
    /// \brief A reference to the parent
    PostRoute& _parent;

    /// \brief A const reference to the sessionid.
    const std::string& _sessionId;

    /// \brief A const reference to the request.
    const Poco::Net::HTTPServerRequest& _request;
    
    /// \brief THe form id.
    const std::string& _formId;
    
};


} } // namespace ofx::HTTP
