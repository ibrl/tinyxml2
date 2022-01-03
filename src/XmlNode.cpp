#include "XmlNode.h"
#include "src/XMLUtil.h"
using namespace tinyxml2;

// --------- XMLNode ----------- //

XMLNode::XMLNode( XMLDocument* doc ) :
    _document( doc ),
    _parent( 0 ),
    _value(),
    _parseLineNum( 0 ),
    _firstChild( 0 ), _lastChild( 0 ),
    _prev( 0 ), _next( 0 ),
    _userData( 0 ),
    _memPool( 0 )
{
}


XMLNode::~XMLNode()
{
    DeleteChildren();
    if ( _parent ) {
        _parent->Unlink( this );
    }
}

const char* XMLNode::Value() const
{
    // Edge case: XMLDocuments don't have a Value. Return null.
    if ( this->ToDocument() )
        return 0;
    return _value.GetStr();
}

void XMLNode::SetValue( const char* str, bool staticMem )
{
    if ( staticMem ) {
        _value.SetInternedStr( str );
    }
    else {
        _value.SetStr( str );
    }
}

XMLNode* XMLNode::DeepClone(XMLDocument* target) const
{
    XMLNode* clone = this->ShallowClone(target);
    if (!clone) return 0;

    for (const XMLNode* child = this->FirstChild(); child; child = child->NextSibling()) {
        XMLNode* childClone = child->DeepClone(target);
        TIXMLASSERT(childClone);
        clone->InsertEndChild(childClone);
    }
    return clone;
}

void XMLNode::DeleteChildren()
{
    while( _firstChild ) {
        TIXMLASSERT( _lastChild );
        DeleteChild( _firstChild );
    }
    _firstChild = _lastChild = 0;
}


void XMLNode::Unlink( XMLNode* child )
{
    TIXMLASSERT( child );
    TIXMLASSERT( child->_document == _document );
    TIXMLASSERT( child->_parent == this );
    if ( child == _firstChild ) {
        _firstChild = _firstChild->_next;
    }
    if ( child == _lastChild ) {
        _lastChild = _lastChild->_prev;
    }

    if ( child->_prev ) {
        child->_prev->_next = child->_next;
    }
    if ( child->_next ) {
        child->_next->_prev = child->_prev;
    }
    child->_next = 0;
    child->_prev = 0;
    child->_parent = 0;
}


void XMLNode::DeleteChild( XMLNode* node )
{
    TIXMLASSERT( node );
    TIXMLASSERT( node->_document == _document );
    TIXMLASSERT( node->_parent == this );
    Unlink( node );
    TIXMLASSERT(node->_prev == 0);
    TIXMLASSERT(node->_next == 0);
    TIXMLASSERT(node->_parent == 0);
    DeleteNode( node );
}


XMLNode* XMLNode::InsertEndChild( XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }
    InsertChildPreamble( addThis );

    if ( _lastChild ) {
        TIXMLASSERT( _firstChild );
        TIXMLASSERT( _lastChild->_next == 0 );
        _lastChild->_next = addThis;
        addThis->_prev = _lastChild;
        _lastChild = addThis;

        addThis->_next = 0;
    }
    else {
        TIXMLASSERT( _firstChild == 0 );
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
    return addThis;
}


XMLNode* XMLNode::InsertFirstChild( XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }
    InsertChildPreamble( addThis );

    if ( _firstChild ) {
        TIXMLASSERT( _lastChild );
        TIXMLASSERT( _firstChild->_prev == 0 );

        _firstChild->_prev = addThis;
        addThis->_next = _firstChild;
        _firstChild = addThis;

        addThis->_prev = 0;
    }
    else {
        TIXMLASSERT( _lastChild == 0 );
        _firstChild = _lastChild = addThis;

        addThis->_prev = 0;
        addThis->_next = 0;
    }
    addThis->_parent = this;
    return addThis;
}


XMLNode* XMLNode::InsertAfterChild( XMLNode* afterThis, XMLNode* addThis )
{
    TIXMLASSERT( addThis );
    if ( addThis->_document != _document ) {
        TIXMLASSERT( false );
        return 0;
    }

    TIXMLASSERT( afterThis );

    if ( afterThis->_parent != this ) {
        TIXMLASSERT( false );
        return 0;
    }
    if ( afterThis == addThis ) {
        // Current state: BeforeThis -> AddThis -> OneAfterAddThis
        // Now AddThis must disappear from it's location and then
        // reappear between BeforeThis and OneAfterAddThis.
        // So just leave it where it is.
        return addThis;
    }

    if ( afterThis->_next == 0 ) {
        // The last node or the only node.
        return InsertEndChild( addThis );
    }
    InsertChildPreamble( addThis );
    addThis->_prev = afterThis;
    addThis->_next = afterThis->_next;
    afterThis->_next->_prev = addThis;
    afterThis->_next = addThis;
    addThis->_parent = this;
    return addThis;
}




const XMLElement* XMLNode::FirstChildElement( const char* name ) const
{
    for( const XMLNode* node = _firstChild; node; node = node->_next ) {
        const XMLElement* element = node->ToElementWithName( name );
        if ( element ) {
            return element;
        }
    }
    return 0;
}


const XMLElement* XMLNode::LastChildElement( const char* name ) const
{
    for( const XMLNode* node = _lastChild; node; node = node->_prev ) {
        const XMLElement* element = node->ToElementWithName( name );
        if ( element ) {
            return element;
        }
    }
    return 0;
}


const XMLElement* XMLNode::NextSiblingElement( const char* name ) const
{
    for( const XMLNode* node = _next; node; node = node->_next ) {
        const XMLElement* element = node->ToElementWithName( name );
        if ( element ) {
            return element;
        }
    }
    return 0;
}


const XMLElement* XMLNode::PreviousSiblingElement( const char* name ) const
{
    for( const XMLNode* node = _prev; node; node = node->_prev ) {
        const XMLElement* element = node->ToElementWithName( name );
        if ( element ) {
            return element;
        }
    }
    return 0;
}


char* XMLNode::ParseDeep( char* p, StrPair* parentEndTag, int* curLineNumPtr )
{
    // This is a recursive method, but thinking about it "at the current level"
    // it is a pretty simple flat list:
    //		<foo/>
    //		<!-- comment -->
    //
    // With a special case:
    //		<foo>
    //		</foo>
    //		<!-- comment -->
    //
    // Where the closing element (/foo) *must* be the next thing after the opening
    // element, and the names must match. BUT the tricky bit is that the closing
    // element will be read by the child.
    //
    // 'endTag' is the end tag for this node, it is returned by a call to a child.
    // 'parentEnd' is the end tag for the parent, which is filled in and returned.

    XMLDocument::DepthTracker tracker(_document);
    if (_document->Error())
        return 0;

    while( p && *p ) {
        XMLNode* node = 0;

        p = _document->Identify( p, &node );
        TIXMLASSERT( p );
        if ( node == 0 ) {
            break;
        }

       const int initialLineNum = node->_parseLineNum;

        StrPair endTag;
        p = node->ParseDeep( p, &endTag, curLineNumPtr );
        if ( !p ) {
            DeleteNode( node );
            if ( !_document->Error() ) {
                _document->SetError( XML_ERROR_PARSING, initialLineNum, 0);
            }
            break;
        }

        const XMLDeclaration* const decl = node->ToDeclaration();
        if ( decl ) {
            // Declarations are only allowed at document level
            //
            // Multiple declarations are allowed but all declarations
            // must occur before anything else.
            //
            // Optimized due to a security test case. If the first node is
            // a declaration, and the last node is a declaration, then only
            // declarations have so far been added.
            bool wellLocated = false;

            if (ToDocument()) {
                if (FirstChild()) {
                    wellLocated =
                        FirstChild() &&
                        FirstChild()->ToDeclaration() &&
                        LastChild() &&
                        LastChild()->ToDeclaration();
                }
                else {
                    wellLocated = true;
                }
            }
            if ( !wellLocated ) {
                _document->SetError( XML_ERROR_PARSING_DECLARATION, initialLineNum, "XMLDeclaration value=%s", decl->Value());
                DeleteNode( node );
                break;
            }
        }

        XMLElement* ele = node->ToElement();
        if ( ele ) {
            // We read the end tag. Return it to the parent.
            if ( ele->ClosingType() == XMLElement::CLOSING ) {
                if ( parentEndTag ) {
                    ele->_value.TransferTo( parentEndTag );
                }
                node->_memPool->SetTracked();   // created and then immediately deleted.
                DeleteNode( node );
                return p;
            }

            // Handle an end tag returned to this level.
            // And handle a bunch of annoying errors.
            bool mismatch = false;
            if ( endTag.Empty() ) {
                if ( ele->ClosingType() == XMLElement::OPEN ) {
                    mismatch = true;
                }
            }
            else {
                if ( ele->ClosingType() != XMLElement::OPEN ) {
                    mismatch = true;
                }
                else if ( !XMLUtil::StringEqual( endTag.GetStr(), ele->Name() ) ) {
                    mismatch = true;
                }
            }
            if ( mismatch ) {
                _document->SetError( XML_ERROR_MISMATCHED_ELEMENT, initialLineNum, "XMLElement name=%s", ele->Name());
                DeleteNode( node );
                break;
            }
        }
        InsertEndChild( node );
    }
    return 0;
}

/*static*/ void XMLNode::DeleteNode( XMLNode* node )
{
    if ( node == 0 ) {
        return;
    }
    TIXMLASSERT(node->_document);
    if (!node->ToDocument()) {
        node->_document->MarkInUse(node);
    }

    MemPool* pool = node->_memPool;
    node->~XMLNode();
    pool->Free( node );
}

void XMLNode::InsertChildPreamble( XMLNode* insertThis ) const
{
    TIXMLASSERT( insertThis );
    TIXMLASSERT( insertThis->_document == _document );

    if (insertThis->_parent) {
        insertThis->_parent->Unlink( insertThis );
    }
    else {
        insertThis->_document->MarkInUse(insertThis);
        insertThis->_memPool->SetTracked();
    }
}

const XMLElement* XMLNode::ToElementWithName( const char* name ) const
{
    const XMLElement* element = this->ToElement();
    if ( element == 0 ) {
        return 0;
    }
    if ( name == 0 ) {
        return element;
    }
    if ( XMLUtil::StringEqual( element->Name(), name ) ) {
       return element;
    }
    return 0;
}

