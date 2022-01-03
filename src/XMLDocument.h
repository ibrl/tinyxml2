#ifndef XMLDOCUMENT_H
#define XMLDOCUMENT_H

#include "src/XmlNode.h"

namespace tinyxml2 {

enum Whitespace {
    PRESERVE_WHITESPACE,
    COLLAPSE_WHITESPACE
};

/** A Document binds together all the functionality.
    It can be saved, loaded, and printed to the screen.
    All Nodes are connected and allocated to a Document.
    If the Document is deleted, all its Nodes are also deleted.
*/
class  XMLDocument : public XMLNode
{
    friend class XMLElement;
    // Gives access to SetError and Push/PopDepth, but over-access for everything else.
    // Wishing C++ had "internal" scope.
    friend class XMLNode;
    friend class XMLText;
    friend class XMLComment;
    friend class XMLDeclaration;
    friend class XMLUnknown;
public:
    /// constructor
    XMLDocument( bool processEntities = true, Whitespace whitespaceMode = PRESERVE_WHITESPACE );
    ~XMLDocument();

    virtual XMLDocument* ToDocument()				{
        TIXMLASSERT( this == _document );
        return this;
    }
    virtual const XMLDocument* ToDocument() const	{
        TIXMLASSERT( this == _document );
        return this;
    }

    /**
        Parse an XML file from a character string.
        Returns XML_SUCCESS (0) on success, or
        an errorID.

        You may optionally pass in the 'nBytes', which is
        the number of bytes which will be parsed. If not
        specified, TinyXML-2 will assume 'xml' points to a
        null terminated string.
    */
    XMLError Parse( const char* xml, size_t nBytes=static_cast<size_t>(-1) );

    /**
        Load an XML file from disk.
        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError LoadFile( const char* filename );

    /**
        Load an XML file from disk. You are responsible
        for providing and closing the FILE*.

        NOTE: The file should be opened as binary ("rb")
        not text in order for TinyXML-2 to correctly
        do newline normalization.

        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError LoadFile( FILE* );

    /**
        Save the XML file to disk.
        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError SaveFile( const char* filename, bool compact = false );

    /**
        Save the XML file to disk. You are responsible
        for providing and closing the FILE*.

        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError SaveFile( FILE* fp, bool compact = false );

    bool ProcessEntities() const		{
        return _processEntities;
    }
    Whitespace WhitespaceMode() const	{
        return _whitespaceMode;
    }

    /**
        Returns true if this document has a leading Byte Order Mark of UTF8.
    */
    bool HasBOM() const {
        return _writeBOM;
    }
    /** Sets whether to write the BOM when writing the file.
    */
    void SetBOM( bool useBOM ) {
        _writeBOM = useBOM;
    }

    /** Return the root element of DOM. Equivalent to FirstChildElement().
        To get the first node, use FirstChild().
    */
    XMLElement* RootElement()				{
        return FirstChildElement();
    }
    const XMLElement* RootElement() const	{
        return FirstChildElement();
    }

    /** Print the Document. If the Printer is not provided, it will
        print to stdout. If you provide Printer, this can print to a file:
        @verbatim
        XMLPrinter printer( fp );
        doc.Print( &printer );
        @endverbatim

        Or you can use a printer to print to memory:
        @verbatim
        XMLPrinter printer;
        doc.Print( &printer );
        // printer.CStr() has a const char* to the XML
        @endverbatim
    */
    void Print( XMLPrinter* streamer=0 ) const;
    virtual bool Accept( XMLVisitor* visitor ) const;

    /**
        Create a new Element associated with
        this Document. The memory for the Element
        is managed by the Document.
    */
    XMLElement* NewElement( const char* name );
    /**
        Create a new Comment associated with
        this Document. The memory for the Comment
        is managed by the Document.
    */
    XMLComment* NewComment( const char* comment );
    /**
        Create a new Text associated with
        this Document. The memory for the Text
        is managed by the Document.
    */
    XMLText* NewText( const char* text );
    /**
        Create a new Declaration associated with
        this Document. The memory for the object
        is managed by the Document.

        If the 'text' param is null, the standard
        declaration is used.:
        @verbatim
            <?xml version="1.0" encoding="UTF-8"?>
        @endverbatim
    */
    XMLDeclaration* NewDeclaration( const char* text=0 );
    /**
        Create a new Unknown associated with
        this Document. The memory for the object
        is managed by the Document.
    */
    XMLUnknown* NewUnknown( const char* text );

    /**
        Delete a node associated with this document.
        It will be unlinked from the DOM.
    */
    void DeleteNode( XMLNode* node );

    /// Clears the error flags.
    void ClearError();

    /// Return true if there was an error parsing the document.
    bool Error() const {
        return _errorID != XML_SUCCESS;
    }
    /// Return the errorID.
    XMLError  ErrorID() const {
        return _errorID;
    }
    const char* ErrorName() const;
    static const char* ErrorIDToName(XMLError errorID);

    /** Returns a "long form" error description. A hopefully helpful
        diagnostic with location, line number, and/or additional info.
    */
    const char* ErrorStr() const;

    /// A (trivial) utility function that prints the ErrorStr() to stdout.
    void PrintError() const;

    /// Return the line where the error occurred, or zero if unknown.
    int ErrorLineNum() const
    {
        return _errorLineNum;
    }

    /// Clear the document, resetting it to the initial state.
    void Clear();

    /**
        Copies this document to a target document.
        The target will be completely cleared before the copy.
        If you want to copy a sub-tree, see XMLNode::DeepClone().

        NOTE: that the 'target' must be non-null.
    */
    void DeepCopy(XMLDocument* target) const;

    // internal
    char* Identify( char* p, XMLNode** node );

    // internal
    void MarkInUse(const XMLNode* const);

    virtual XMLNode* ShallowClone( XMLDocument* /*document*/ ) const	{
        return 0;
    }
    virtual bool ShallowEqual( const XMLNode* /*compare*/ ) const	{
        return false;
    }

private:
    XMLDocument( const XMLDocument& );	// not supported
    void operator=( const XMLDocument& );	// not supported

    bool			_writeBOM;
    bool			_processEntities;
    XMLError		_errorID;
    Whitespace		_whitespaceMode;
    mutable StrPair	_errorStr;
    int             _errorLineNum;
    char*			_charBuffer;
    int				_parseCurLineNum;
    int				_parsingDepth;
    // Memory tracking does add some overhead.
    // However, the code assumes that you don't
    // have a bunch of unlinked nodes around.
    // Therefore it takes less memory to track
    // in the document vs. a linked list in the XMLNode,
    // and the performance is the same.
    DynArray<XMLNode*, 10> _unlinked;

    MemPoolT< sizeof(XMLElement) >	 _elementPool;
    MemPoolT< sizeof(XMLAttribute) > _attributePool;
    MemPoolT< sizeof(XMLText) >		 _textPool;
    MemPoolT< sizeof(XMLComment) >	 _commentPool;

    static const char* _errorNames[XML_ERROR_COUNT];

    void Parse();

    void SetError( XMLError error, int lineNum, const char* format, ... );

    // Something of an obvious security hole, once it was discovered.
    // Either an ill-formed XML or an excessively deep one can overflow
    // the stack. Track stack depth, and error out if needed.
    class DepthTracker {
    public:
        explicit DepthTracker(XMLDocument * document) {
            this->_document = document;
            document->PushDepth();
        }
        ~DepthTracker() {
            _document->PopDepth();
        }
    private:
        XMLDocument * _document;
    };
    void PushDepth();
    void PopDepth();

    template<class NodeType, int PoolElementSize>
    NodeType* CreateUnlinkedNode( MemPoolT<PoolElementSize>& pool );
};

template<class NodeType, int PoolElementSize>
inline NodeType* XMLDocument::CreateUnlinkedNode( MemPoolT<PoolElementSize>& pool )
{
    TIXMLASSERT( sizeof( NodeType ) == PoolElementSize );
    TIXMLASSERT( sizeof( NodeType ) == pool.ItemSize() );
    NodeType* returnNode = new (pool.Alloc()) NodeType( this );
    TIXMLASSERT( returnNode );
    returnNode->_memPool = &pool;

    _unlinked.Push(returnNode);
    return returnNode;
}

/** A Document binds together all the functionality.
    It can be saved, loaded, and printed to the screen.
    All Nodes are connected and allocated to a Document.
    If the Document is deleted, all its Nodes are also deleted.
*/
class TINYXML2_LIB XMLDocument : public XMLNode
{
    friend class XMLElement;
    // Gives access to SetError and Push/PopDepth, but over-access for everything else.
    // Wishing C++ had "internal" scope.
    friend class XMLNode;
    friend class XMLText;
    friend class XMLComment;
    friend class XMLDeclaration;
    friend class XMLUnknown;
public:
    /// constructor
    XMLDocument( bool processEntities = true, Whitespace whitespaceMode = PRESERVE_WHITESPACE );
    ~XMLDocument();

    virtual XMLDocument* ToDocument()				{
        TIXMLASSERT( this == _document );
        return this;
    }
    virtual const XMLDocument* ToDocument() const	{
        TIXMLASSERT( this == _document );
        return this;
    }

    /**
        Parse an XML file from a character string.
        Returns XML_SUCCESS (0) on success, or
        an errorID.

        You may optionally pass in the 'nBytes', which is
        the number of bytes which will be parsed. If not
        specified, TinyXML-2 will assume 'xml' points to a
        null terminated string.
    */
    XMLError Parse( const char* xml, size_t nBytes=static_cast<size_t>(-1) );

    /**
        Load an XML file from disk.
        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError LoadFile( const char* filename );

    /**
        Load an XML file from disk. You are responsible
        for providing and closing the FILE*.

        NOTE: The file should be opened as binary ("rb")
        not text in order for TinyXML-2 to correctly
        do newline normalization.

        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError LoadFile( FILE* );

    /**
        Save the XML file to disk.
        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError SaveFile( const char* filename, bool compact = false );

    /**
        Save the XML file to disk. You are responsible
        for providing and closing the FILE*.

        Returns XML_SUCCESS (0) on success, or
        an errorID.
    */
    XMLError SaveFile( FILE* fp, bool compact = false );

    bool ProcessEntities() const		{
        return _processEntities;
    }
    Whitespace WhitespaceMode() const	{
        return _whitespaceMode;
    }

    /**
        Returns true if this document has a leading Byte Order Mark of UTF8.
    */
    bool HasBOM() const {
        return _writeBOM;
    }
    /** Sets whether to write the BOM when writing the file.
    */
    void SetBOM( bool useBOM ) {
        _writeBOM = useBOM;
    }

    /** Return the root element of DOM. Equivalent to FirstChildElement().
        To get the first node, use FirstChild().
    */
    XMLElement* RootElement()				{
        return FirstChildElement();
    }
    const XMLElement* RootElement() const	{
        return FirstChildElement();
    }

    /** Print the Document. If the Printer is not provided, it will
        print to stdout. If you provide Printer, this can print to a file:
        @verbatim
        XMLPrinter printer( fp );
        doc.Print( &printer );
        @endverbatim

        Or you can use a printer to print to memory:
        @verbatim
        XMLPrinter printer;
        doc.Print( &printer );
        // printer.CStr() has a const char* to the XML
        @endverbatim
    */
    void Print( XMLPrinter* streamer=0 ) const;
    virtual bool Accept( XMLVisitor* visitor ) const;

    /**
        Create a new Element associated with
        this Document. The memory for the Element
        is managed by the Document.
    */
    XMLElement* NewElement( const char* name );
    /**
        Create a new Comment associated with
        this Document. The memory for the Comment
        is managed by the Document.
    */
    XMLComment* NewComment( const char* comment );
    /**
        Create a new Text associated with
        this Document. The memory for the Text
        is managed by the Document.
    */
    XMLText* NewText( const char* text );
    /**
        Create a new Declaration associated with
        this Document. The memory for the object
        is managed by the Document.

        If the 'text' param is null, the standard
        declaration is used.:
        @verbatim
            <?xml version="1.0" encoding="UTF-8"?>
        @endverbatim
    */
    XMLDeclaration* NewDeclaration( const char* text=0 );
    /**
        Create a new Unknown associated with
        this Document. The memory for the object
        is managed by the Document.
    */
    XMLUnknown* NewUnknown( const char* text );

    /**
        Delete a node associated with this document.
        It will be unlinked from the DOM.
    */
    void DeleteNode( XMLNode* node );

    /// Clears the error flags.
    void ClearError();

    /// Return true if there was an error parsing the document.
    bool Error() const {
        return _errorID != XML_SUCCESS;
    }
    /// Return the errorID.
    XMLError  ErrorID() const {
        return _errorID;
    }
    const char* ErrorName() const;
    static const char* ErrorIDToName(XMLError errorID);

    /** Returns a "long form" error description. A hopefully helpful
        diagnostic with location, line number, and/or additional info.
    */
    const char* ErrorStr() const;

    /// A (trivial) utility function that prints the ErrorStr() to stdout.
    void PrintError() const;

    /// Return the line where the error occurred, or zero if unknown.
    int ErrorLineNum() const
    {
        return _errorLineNum;
    }

    /// Clear the document, resetting it to the initial state.
    void Clear();

    /**
        Copies this document to a target document.
        The target will be completely cleared before the copy.
        If you want to copy a sub-tree, see XMLNode::DeepClone().

        NOTE: that the 'target' must be non-null.
    */
    void DeepCopy(XMLDocument* target) const;

    // internal
    char* Identify( char* p, XMLNode** node );

    // internal
    void MarkInUse(const XMLNode* const);

    virtual XMLNode* ShallowClone( XMLDocument* /*document*/ ) const	{
        return 0;
    }
    virtual bool ShallowEqual( const XMLNode* /*compare*/ ) const	{
        return false;
    }

private:
    XMLDocument( const XMLDocument& );	// not supported
    void operator=( const XMLDocument& );	// not supported

    bool			_writeBOM;
    bool			_processEntities;
    XMLError		_errorID;
    Whitespace		_whitespaceMode;
    mutable StrPair	_errorStr;
    int             _errorLineNum;
    char*			_charBuffer;
    int				_parseCurLineNum;
    int				_parsingDepth;
    // Memory tracking does add some overhead.
    // However, the code assumes that you don't
    // have a bunch of unlinked nodes around.
    // Therefore it takes less memory to track
    // in the document vs. a linked list in the XMLNode,
    // and the performance is the same.
    DynArray<XMLNode*, 10> _unlinked;

    MemPoolT< sizeof(XMLElement) >	 _elementPool;
    MemPoolT< sizeof(XMLAttribute) > _attributePool;
    MemPoolT< sizeof(XMLText) >		 _textPool;
    MemPoolT< sizeof(XMLComment) >	 _commentPool;

    static const char* _errorNames[XML_ERROR_COUNT];

    void Parse();

    void SetError( XMLError error, int lineNum, const char* format, ... );

    // Something of an obvious security hole, once it was discovered.
    // Either an ill-formed XML or an excessively deep one can overflow
    // the stack. Track stack depth, and error out if needed.
    class DepthTracker {
    public:
        explicit DepthTracker(XMLDocument * document) {
            this->_document = document;
            document->PushDepth();
        }
        ~DepthTracker() {
            _document->PopDepth();
        }
    private:
        XMLDocument * _document;
    };
    void PushDepth();
    void PopDepth();

    template<class NodeType, int PoolElementSize>
    NodeType* CreateUnlinkedNode( MemPoolT<PoolElementSize>& pool );
};

template<class NodeType, int PoolElementSize>
inline NodeType* XMLDocument::CreateUnlinkedNode( MemPoolT<PoolElementSize>& pool )
{
    TIXMLASSERT( sizeof( NodeType ) == PoolElementSize );
    TIXMLASSERT( sizeof( NodeType ) == pool.ItemSize() );
    NodeType* returnNode = new (pool.Alloc()) NodeType( this );
    TIXMLASSERT( returnNode );
    returnNode->_memPool = &pool;

    _unlinked.Push(returnNode);
    return returnNode;
}

}

#endif // XMLDOCUMENT_H
