//
// $Id$
//

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    gen_bounded_obj_sequence_ci.cpp
//
// = DESCRIPTION
//    Visitor generating code for Sequence in the client inline
//
// = AUTHOR
//    Michael Kircher
//
//    Modifications by Aniruddha Gokhale
// ============================================================================

#include	"be.h"

#include "be_visitor_sequence.h"

ACE_RCSID(be_visitor_sequence, gen_bounded_obj_sequence_ci, "$Id$")


int
be_visitor_sequence_ci::gen_bounded_obj_sequence (be_sequence *node)
{
  TAO_OutStream *os = this->ctx_->stream ();
  be_type *bt;

  // retrieve the base type since we may need to do some code
  // generation for the base type.
  bt = be_type::narrow_from_decl (node->base_type ());
  if (!bt)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                         "(%N:%l) be_visitor_sequence_ci::"
                         "visit_sequence - "
                         "Bad element type\n"), -1);
    }

  // generate the class name
  be_type  *pt; // base types

  if (bt->node_type () == AST_Decl::NT_typedef)
    {
      // get the primitive base type of this typedef node
      be_typedef *t = be_typedef::narrow_from_decl (bt);
      pt = t->primitive_base_type ();
    }
  else
    pt = bt;

  const char * class_name = node->instance_name ();

  static char full_class_name [NAMEBUFSIZE];
  ACE_OS::memset (full_class_name, '\0', NAMEBUFSIZE);

  if (node->is_nested ())
    {
      ACE_OS::sprintf (full_class_name, "%s::%s",
                       be_scope::narrow_from_scope (node->defined_in ())->decl ()->full_name (),
                       class_name);
    }
  else
    {
      ACE_OS::sprintf (full_class_name, "%s",
                       class_name);
    }

  be_visitor_context ctx (*this->ctx_);
  ctx.state (TAO_CodeGen::TAO_SEQUENCE_BASE_CI);
  be_visitor *visitor = tao_cg->make_visitor (&ctx);

  // !! branching in either compile time template instantiation
  // or manual template instatiation
  os->gen_ifdef_AHETI();

  os->gen_ifdef_macro (class_name);

  os->indent ();

  // first generate the static methods since they are used by others. Sinc
  // ethey are inlined, their definition needs to come before their use else
  // some compilers give lost of warnings.

  // allocbuf
  *os << "ACE_INLINE "; bt->accept (visitor);
  *os << " **" << be_nl;
  *os << full_class_name << "::allocbuf (CORBA::ULong /* length */) "
      << "// Allocate storage for a sequence.." << be_nl
      << "{" << be_idt_nl;
  // the accept is here the first time used and if an
  // error occurs, it will occur here. Later no check
  // for errors will be done.
  if (bt->accept (visitor) == -1)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                         "(%N:%l) be_visitor_sequence_ci::"
                         "visit_sequence - "
                         "base type visit failed\n"),
                        -1);
    }
  *os <<" **buf = 0;" << be_nl
      << be_nl
      << "ACE_NEW_RETURN (buf, ";
  bt->accept (visitor);
  *os << "*[" << node->max_size () << "], 0);" << be_nl
      << be_nl
      << "for (CORBA::ULong i = 0; i < " << node->max_size () << "; i++)" << be_idt_nl
      << "buf[i] = ";
  bt->accept (visitor);
  *os << "::_nil ();" << be_uidt_nl
      << be_nl
      << "return buf;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // freebuf
  *os << "ACE_INLINE void" << be_nl
      << full_class_name << "::freebuf (";
  bt->accept (visitor);
  *os << " **buffer)" << be_nl
      << "{" << be_idt_nl
      << "for (CORBA::ULong i = 0; i < " << node->max_size () << "; ++i)" << be_idt_nl
      << "if (buffer[i] != ";
  bt->accept (visitor);
  *os << "::_nil ())" << be_nl
      << "{" << be_idt_nl
      << "CORBA::release (buffer[i]);" << be_nl
      << "buffer[i] = ";
  bt->accept (visitor);
  *os << "::_nil ();" << be_uidt_nl
      << "}" << be_uidt_nl
      << be_nl
      << "delete[] buffer;" << be_uidt_nl
      << "} " << be_nl
      << be_nl;

  // constructor
  *os << "// default ctor" << be_nl;
  *os << "ACE_INLINE" << be_nl
      << full_class_name << "::" << class_name << " (void)" << be_idt_nl
      << "  :  TAO_Bounded_Base_Sequence (" << node->max_size ()
      << ", 0)" << be_uidt_nl
      << "{" << be_nl
      << "}" << be_nl
      << be_nl;

  // constructor
  *os << "ACE_INLINE" << be_nl
      << full_class_name << "::" << class_name
      << " (CORBA::ULong length," << be_idt_nl;
  bt->accept (visitor);
  *os <<"* *value," << be_nl
      << "CORBA::Boolean release)" << be_uidt_nl
      << "// Constructor from data." << be_nl
      << "  : TAO_Bounded_Base_Sequence (" << node->max_size () << ", length, value, release)" << be_nl
      << "{" << be_nl
      << "}" << be_nl
      << be_nl;

  // constructor
  *os << "ACE_INLINE" << be_nl
      << full_class_name << "::" << class_name << " (const "
      << class_name << " &rhs)" << be_nl
      << "// Copy constructor." << be_idt_nl
      << ": TAO_Bounded_Base_Sequence (rhs)" << be_uidt_nl
      << "{" << be_idt_nl
      << "if (rhs.buffer_ != 0)" << be_nl
      << "{" << be_idt_nl;
  bt->accept(visitor);
  *os <<" **tmp1 = allocbuf (" << node->max_size () << ");" << be_nl;
  bt->accept(visitor);
  *os <<" ** const tmp2 = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << "** ACE_CAST_CONST, rhs.buffer_);" << be_nl
      << "for (CORBA::ULong i = 0; i < rhs.length_; i++)" << be_idt_nl
      << "tmp1[i] = "; 
  bt->accept (visitor); 
  *os << "::_duplicate (tmp2[i]);" << be_uidt_nl
      << "this->buffer_ = tmp1;" << be_uidt_nl
      << "}" << be_nl
      << "else" << be_nl
      << "{" << be_idt_nl
      << "this->buffer_ = 0;" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // assignment operator
  *os << "ACE_INLINE " << full_class_name << " &" << be_nl
      << full_class_name << "::operator= (const "
      << class_name << " &rhs)" << be_nl
      << "// Assignment from another Bounded sequence." << be_nl
      << "{" << be_idt_nl
      << "if (this == &rhs)" << be_idt_nl
      << "return *this;" << be_uidt_nl
      << be_nl
      << "if (this->release_ && this->buffer_ != 0)" << be_nl
      << "{" << be_idt_nl;
  bt->accept(visitor);
  *os <<" **tmp = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << " **, this->buffer_);" << be_nl
      << be_nl
      << "for (CORBA::ULong i = 0; i < this->length_; ++i)" << be_nl
      << "{" << be_idt_nl
      << "CORBA::release (tmp[i]);" << be_nl
      << "tmp[i] = ";
  bt->accept (visitor);
  *os << "::_nil ();" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << "else" << be_idt_nl
      << "this->buffer_ = " << class_name 
      << "::allocbuf (rhs.maximum_);" << be_uidt_nl
      << be_nl
      << "TAO_Bounded_Base_Sequence::operator= (rhs);" << be_nl
      << be_nl;
  bt->accept(visitor);
  *os <<" **tmp1 = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << " **, this->buffer_);" << be_nl;
  bt->accept(visitor);
  *os <<" ** const tmp2 = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << " ** ACE_CAST_CONST, rhs.buffer_);" << be_nl
      << "for (CORBA::ULong i=0; i < rhs.length_; ++i)" << be_idt_nl
      << "tmp1[i] = ";
  bt->accept (visitor);
  *os << "::_duplicate (tmp2[i]);" << be_uidt_nl
      << "return *this;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // operator[]
  be_predefined_type *prim = be_predefined_type::narrow_from_decl (pt);
  int is_pseudo_object =
    pt->node_type () == AST_Decl::NT_pre_defined
    && prim && prim->pt () == AST_PredefinedType::PT_pseudo
    && ACE_OS::strcmp (prim->local_name ()->get_string (),
                       "Object") != 0;

  // operator[]
  if (is_pseudo_object)
    {
      *os << "ACE_INLINE TAO_Pseudo_Object_Manager<";
    }
  else
    {
      *os << "ACE_INLINE TAO_Object_Manager<";
    }

  *os << bt->name () << ","
      << bt->name () << "_var>" << be_nl
      << full_class_name << "::operator[] (CORBA::ULong index) const"
      << " // Read-write accessor." << be_nl
      << "{" << be_idt_nl
      << "ACE_ASSERT (index < this->maximum_);" << be_nl;

  bt->accept(visitor);
  *os <<" **const tmp = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << " ** ACE_CAST_CONST, this->buffer_);" << be_nl;

  if (is_pseudo_object)
    {
      *os << "return TAO_Pseudo_Object_Manager<";
    }
  else
    {
      *os << "return TAO_Object_Manager<";
    }

  *os << bt->name () << ","
      << bt->name () << "_var> "
      << "(tmp + index, this->release_);" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // get_buffer
  *os << "ACE_INLINE ";
  bt->accept(visitor);
  *os << " **" << be_nl;
  *os << full_class_name << "::get_buffer (CORBA::Boolean orphan)" << be_nl
      << "{" << be_idt_nl;
  bt->accept(visitor);
  *os << " **result = 0;" << be_nl
      << "if (orphan == 0)" << be_nl
      << "{" << be_idt_nl
      << "// We retain ownership." << be_nl
      << "if (this->buffer_ == 0)" << be_nl
      << "{" << be_idt_nl
      << "result = " << class_name << "::allocbuf (this->maximum_);" << be_nl
      << "this->buffer_ = result;" << be_nl
      << "this->release_ = 1;" << be_uidt_nl
      << "}" << be_nl
      << "else" << be_nl
      << "{" << be_idt_nl
      << "result = ACE_reinterpret_cast (";
  bt->accept (visitor);
  *os << "**, this->buffer_);" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << "else // if (orphan == 1)" << be_nl
      << "{" << be_idt_nl
      << "if (this->release_ != 0)" << be_nl
      << "{" << be_idt_nl
      << "// We set the state back to default and relinquish" << be_nl
      << "// ownership." << be_nl
      << "result = ACE_reinterpret_cast(";
  bt->accept (visitor);
  *os << "**,this->buffer_);" << be_nl
      << "this->maximum_ = 0;" << be_nl
      << "this->length_ = 0;" << be_nl
      << "this->buffer_ = 0;" << be_nl
      << "this->release_ = 0;" << be_uidt_nl
      << "}" << be_uidt_nl
      << "}" << be_nl
      << "return result;" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  // get_buffer
  *os << "ACE_INLINE const ";
  bt->accept (visitor);
  *os << "* *" << be_nl
      << full_class_name << "::get_buffer (void) const" << be_nl
      << "{" << be_idt_nl
      << "return ACE_reinterpret_cast(const ";
  bt->accept (visitor);
  *os << " ** ACE_CAST_CONST, this->buffer_);" << be_uidt_nl
      << "}" << be_nl
      << be_nl;

  os->gen_endif ();

  // generate #endif for AHETI
  os->gen_endif_AHETI();

  delete visitor;
  return 0;
}
