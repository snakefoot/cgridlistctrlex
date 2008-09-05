#pragma once

//------------------------------------------------------------------------
// Author:  Rolf Kristensen	
// Source:  http://www.codeproject.com/KB/list/CGridListCtrlEx.aspx
// License: Free to use for all
//
//! CGridColumnTraitVisitor enables the use of the visitor-pattern to
//! add extra behavior to the CGridColumnTrait classes
//------------------------------------------------------------------------

class CGridColumnTrait;
class CGridColumnTraitCombo;
class CGridColumnTraitEdit;
class CGridColumnTraitText;

class CGridColumnTraitVisitor
{
public:
	void Visit(CGridColumnTrait& item)		{}
	void Visit(CGridColumnTraitCombo& item)	{}
	void Visit(CGridColumnTraitEdit& item)	{}
	void Visit(CGridColumnTraitText& item)	{}
};