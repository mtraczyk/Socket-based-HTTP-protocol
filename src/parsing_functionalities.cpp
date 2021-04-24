#include "parsing_functionalities.h"
#include <string>

constexpr static size_t firstIndexOfAString = 0;

namespace HTTPRequestPatterns {
  using std::regex;

  /* !! I interpret OWS as an arbitrary white space character.
   That was the case during any other project at this university. !! */

  // Regex for a request line that is supported by this server.
  regex supportedRequestLine(R"((GET|HEAD)\s\/[a-zA-Z0-9.\-\/]*\s(HTTP\/1\.1))");

  /* Regex for unsupported request lines such as "GeT / HTTP/1.1" or "asd12^& file.txt HTTP/1.1"
   * Error code 501 should be return when such a line is encountered.
   */
  regex unsupportedRequestLine(R"(^(?!GET\s|HEAD\s)\S+\s\S*\s(HTTP\/1\.1))");

  // Regex for a supported connection header, IT IS CASE SENSITIVE.
  regex headerConnection(R"((C|c)(onnection):(\s)*(close|keep\-alive)(\s)*)");

  /* Regex for a supported Content-Length header, IT IS CASE SENSITIVE. The field value can only equal to zero.
   * I interpret "0000..." where "..." implies arbitrary number of zero digits as zero.
   */
  regex contentLength(R"((Content\-Length):(\s)*(0)+(\s)*)");

  // A field value of a proper Content-Length header must equal to zero in this server.
  regex incorrectContentLength(R"((Content\-Length):(\s)*.*(\s)*)");

  // Regex for any other header of a correct format.
  regex unsupportedHeader(R"(.+:(\s)*.*(\s)*)");
}

namespace {
  constexpr size_t lengthOfGETString = 3; // Length of "GET".

  /* Auxiliary function used to obtain type of a request from a correct request line.
   * There are only two supported request types: "GET" and "HEAD".
   */
  HTTPRequestParser::parserCodesType getRequestType(std::string const &requestLine) {
    if (requestLine.substr(firstIndexOfAString, lengthOfGETString) == "GET") {
      return HTTPRequestParser::requestGET;
    }

    /* If the first three characters don't represent "GET" then it must be a HEAD request as we know
     * that requestLine is a supported request line.
     */
    return HTTPRequestParser::requestHEAD;
  }

  // Auxiliary function used to obtain connection type from a proper connection header.
  HTTPRequestParser::parserCodesType getConnectionType(std::string const &connectionHeaderLine) {
    auto index = connectionHeaderLine.find(':') + 1; // OWS starting index.
    std::string connectionTypeString; // String to store connection type in it.

    while (isblank(connectionHeaderLine[index])) {
      index++;
    }

    /* There is no need to check for '-' character even though it is present in "keep-alive" string.
     * Simply because if it is not "close" then it must be "keep-alive".
     */
    while (std::isalpha(connectionHeaderLine[index]) && index < connectionHeaderLine.size()) {
      connectionTypeString += connectionHeaderLine[index];
      index++;
    }

    if (connectionTypeString == "close") {
      return HTTPRequestParser::connectionClose;
    }

    return HTTPRequestParser::connectionKeepAlive;
  }
}

void getInfoAboutParsedLine(HTTPRequestParser *requestParserInstance, uint8_t requestType) noexcept {
  if (requestParserInstance->requestType) {
    // There can't be more than one request line.
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  } else {
    requestParserInstance->requestType = requestType;
    requestParserInstance->resourcePath.clear();

    auto const &currentLine = requestParserInstance->currentLine;
    // Every path must start with a '/' character.
    for (auto i = currentLine.find('/'); !isblank(currentLine[i]); i++) {
      requestParserInstance->resourcePath += currentLine[i];
    }
  }
}

void processAParsedLine(HTTPRequestParser *requestParserInstance) {
  requestParserInstance->lineParsed = true; // Line was correctly parsed.
  bool hasRequestOccurred = (requestParserInstance->requestType != HTTPRequestParser::noRequest);
  auto const &currentLine = requestParserInstance->currentLine;

  if (std::regex_match(currentLine, HTTPRequestPatterns::supportedRequestLine)) {
    auto requestType = getRequestType(currentLine); // Check if it is a GET or HEAD request.
    getInfoAboutParsedLine(requestParserInstance, requestType);
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::headerConnection) && hasRequestOccurred) {
    // If the same supported header occurred twice then it is an error 400.
    requestParserInstance->errorOccurred = requestParserInstance->connectionHeaderOccurred;
    setWrongFormatError(requestParserInstance); // Set the error if it needs to be set.
    requestParserInstance->connection = getConnectionType(currentLine);
    requestParserInstance->connectionHeaderOccurred = true;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::contentLength) && hasRequestOccurred) {
    // If the same supported header occurred twice then it is an error 400.
    requestParserInstance->errorOccurred = requestParserInstance->contentLengthHeaderOccurred;
    setWrongFormatError(requestParserInstance); // Set the error if it needs to be set.
    requestParserInstance->contentLengthHeaderOccurred = true;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::unsupportedRequestLine)) {
    // Unsupported functionality.
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::unsupportedFunctionalityError;
  } else if (std::regex_match(currentLine, HTTPRequestPatterns::incorrectContentLength)) {
    // Content-Length's value field doesn't equal zero.
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  } else if (!std::regex_match(currentLine, HTTPRequestPatterns::unsupportedHeader)) {
    // Unsupported headers are ignored but the currentLine is of a different format.
    requestParserInstance->errorOccurred = true;
    requestParserInstance->errorType = HTTPRequestParser::wrongFormatError;
  }

  requestParserInstance->currentLine.clear(); // Line was parsed, needs to be cleared.
}

void HTTPRequestParser::parsePartOfARequest(std::string const &requestPart) {
  nextPartOfARequest += requestPart;
  auto positionOfCRLF = nextPartOfARequest.find("\r\n", nextPartOfARequestsIndexPosition);

  if (positionOfCRLF == std::string::npos) {
    /* CRLF wasn't found. It means the line is in parts. The other parts of it need to be read
     * before continuing interpreting it.
    */
    lineParsed = false;
    // When CRLF is broken in two halves, it might become problematic therefore simply skip such a situation.
    if (nextPartOfARequest[nextPartOfARequest.size() - 1] != '\r') {
      prepareForParsingNextLine();
    }
  } else {
    if (currentLine.empty() && nextPartOfARequestsIndexPosition == positionOfCRLF) {
      // The CRLF which ends header section is read.
      nextPartOfARequestsIndexPosition += sizeOfCRLFBlock;

      if (requestType) {
        // Request line was read and all of the other lines didn't provoke any errors.
        lineParsed = true;
        requestParsed = true;
      } else {
        errorOccurred = true;
      }

      return;
    }

    // currentLine has to be updated.
    currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition,
                                             positionOfCRLF - nextPartOfARequestsIndexPosition);
    // Line was read, it needs to be processed now.
    processAParsedLine(this);
    nextPartOfARequestsIndexPosition = positionOfCRLF + sizeOfCRLFBlock; // Where to look next in the buffer.
  }
}

// Auxiliary function to set wrong format error.
inline void setWrongFormatError(HTTPRequestParser *parserInstance) noexcept {
  if (parserInstance->errorOccurred) {
    parserInstance->errorType = HTTPRequestParser::wrongFormatError;
  }
}

bool HTTPRequestParser::isALineParsed() const noexcept {
  return lineParsed;
}

bool HTTPRequestParser::hasAnErrorOccurred() const noexcept {
  return errorOccurred;
}

std::pair<bool, HTTPRequestParser::requestInfo> HTTPRequestParser::getFullyParsedRequest() noexcept {
  finalRequestInfo = {requestType, connection, resourcePath};

  return {requestParsed, finalRequestInfo};
}

HTTPRequestParser::errorCodeType HTTPRequestParser::getErrorType() const noexcept {
  return errorType;
}

void HTTPRequestParser::prepareForParsingNextLine() noexcept {
  /* Last character of nextPartOfARequest equals to '/r' therefore we can move all of the remaining
   * characters from the buffer to the currentLine.
   */
  currentLine += nextPartOfARequest.substr(nextPartOfARequestsIndexPosition);
  nextPartOfARequest.clear();
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
}

void HTTPRequestParser::cleanAfterParsingWholeRequest() noexcept {
  lineParsed = false;
  errorOccurred = false;
  requestParsed = false;
  contentLengthHeaderOccurred = false;
  connectionHeaderOccurred = false;
  requestType = noRequest;
  connection = connectionKeepAlive;
  errorType = noError;
}

void HTTPRequestParser::reset() noexcept {
  currentLine.clear();
  nextPartOfARequest.clear();
  resourcePath.clear();
  lineParsed = false;
  errorOccurred = false;
  requestParsed = false;
  contentLengthHeaderOccurred = false;
  connectionHeaderOccurred = false;
  nextPartOfARequestsIndexPosition = firstIndexOfAString;
  requestType = noRequest;
  connection = connectionKeepAlive;
  errorType = noError;
}




