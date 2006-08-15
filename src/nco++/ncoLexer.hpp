#ifndef INC_ncoLexer_hpp_
#define INC_ncoLexer_hpp_

#include <antlr/config.hpp>
/* $ANTLR 2.7.6 (20060511): "ncoGrammer.g" -> "ncoLexer.hpp"$ */
#include <antlr/CommonToken.hpp>
#include <antlr/InputBuffer.hpp>
#include <antlr/BitSet.hpp>
#include "ncoParserTokenTypes.hpp"
#include <antlr/CharScanner.hpp>
#line 1 "ncoGrammer.g"

    #include <math.h>
    #include <malloc.h>
    #include <assert.h>
    #include <ctype.h>
    #include <iostream>
    #include <string>
    #include <sstream>
    #include "ncap2.hh"
    #include "NcapVar.hh"
    #include "NcapVarVector.hh"
    ANTLR_USING_NAMESPACE(std);
    ANTLR_USING_NAMESPACE(antlr);
    

#line 28 "ncoLexer.hpp"
class CUSTOM_API ncoLexer : public ANTLR_USE_NAMESPACE(antlr)CharScanner, public ncoParserTokenTypes
{
#line 204 "ncoGrammer.g"


private:
    prs_sct *prs_arg;
public:
    // Customized constructor !!
   ncoLexer(ANTLR_USE_NAMESPACE(std)istream& in, prs_sct *prs_in )
   : ANTLR_USE_NAMESPACE(antlr)CharScanner(new ANTLR_USE_NAMESPACE(antlr)CharBuffer(in),true)
   {
        prs_arg=prs_in;
        // This shouldn't really be here 
        // fxm:: should call default constructor
	    initLiterals();
   }



#line 32 "ncoLexer.hpp"
private:
	void initLiterals();
public:
	bool getCaseSensitiveLiterals() const
	{
		return true;
	}
public:
	ncoLexer(ANTLR_USE_NAMESPACE(std)istream& in);
	ncoLexer(ANTLR_USE_NAMESPACE(antlr)InputBuffer& ib);
	ncoLexer(const ANTLR_USE_NAMESPACE(antlr)LexerSharedInputState& state);
	ANTLR_USE_NAMESPACE(antlr)RefToken nextToken();
	public: void mASSIGN(bool _createToken);
	public: void mPLUS_ASSIGN(bool _createToken);
	public: void mMINUS_ASSIGN(bool _createToken);
	public: void mTIMES_ASSIGN(bool _createToken);
	public: void mDIVIDE_ASSIGN(bool _createToken);
	public: void mINC(bool _createToken);
	public: void mDEC(bool _createToken);
	public: void mQUESTION(bool _createToken);
	public: void mLPAREN(bool _createToken);
	public: void mRPAREN(bool _createToken);
	public: void mLCURL(bool _createToken);
	public: void mRCURL(bool _createToken);
	public: void mLSQUARE(bool _createToken);
	public: void mRSQUARE(bool _createToken);
	public: void mCOMMA(bool _createToken);
	public: void mQUOTE(bool _createToken);
	public: void mSEMI(bool _createToken);
	public: void mCOLON(bool _createToken);
	public: void mCARET(bool _createToken);
	public: void mTIMES(bool _createToken);
	public: void mDIVIDE(bool _createToken);
	public: void mMOD(bool _createToken);
	public: void mPLUS(bool _createToken);
	public: void mMINUS(bool _createToken);
	public: void mEQ(bool _createToken);
	public: void mNEQ(bool _createToken);
	public: void mLTHAN(bool _createToken);
	public: void mGTHAN(bool _createToken);
	public: void mLEQ(bool _createToken);
	public: void mGEQ(bool _createToken);
	public: void mLAND(bool _createToken);
	public: void mLNOT(bool _createToken);
	public: void mLOR(bool _createToken);
	protected: void mDGT(bool _createToken);
	protected: void mLPH(bool _createToken);
	protected: void mLPHDGT(bool _createToken);
	protected: void mXPN(bool _createToken);
	protected: void mBLASTOUT(bool _createToken);
	public: void mWhitespace(bool _createToken);
	public: void mCPP_COMMENT(bool _createToken);
	public: void mC_COMMENT(bool _createToken);
	public: void mNUMBER(bool _createToken);
	public: void mNUMBER_DOT(bool _createToken);
	public: void mVAR_ATT(bool _createToken);
	public: void mDIM_VAL(bool _createToken);
	public: void mNSTRING(bool _createToken);
private:
	
	static const unsigned long _tokenSet_0_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_0;
	static const unsigned long _tokenSet_1_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_1;
	static const unsigned long _tokenSet_2_data_[];
	static const ANTLR_USE_NAMESPACE(antlr)BitSet _tokenSet_2;
};

#endif /*INC_ncoLexer_hpp_*/
