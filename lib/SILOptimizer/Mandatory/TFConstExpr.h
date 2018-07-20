//===--- TFConstExpr.h - TensorFlow constant expressions --------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This defines an interface to evaluate Swift language level constant
// expressions.  Its model is intended to be general and reasonably powerful,
// with the goal of standardization in a future version of Swift.
//
// Constant expressions are functions without side effects that take constant
// values and return constant values.  These constants may be integer, floating
// point, and string values, or arrays thereof (up to 1024 elements).  We allow
// abstractions to be built out of fragile structs and tuples.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SILOPTIMIZER_TF_CONSTEXPR_H
#define SWIFT_SILOPTIMIZER_TF_CONSTEXPR_H

#include "swift/Basic/LLVM.h"
#include "llvm/Support/Allocator.h"

namespace swift {
  class ApplyInst;
  class SILInstruction;
  class SILModule;
  class SILValue;
  class SymbolicValue;

namespace tf {

/// This class is the main entrypoint for evaluating constant expressions.  It
/// also handles caching of previously computed constexpr results.
class ConstExprEvaluator {
  /// This is a long-lived bump pointer allocator that holds the arguments and
  /// result values for the cached constexpr calls we have already analyzed.
  llvm::BumpPtrAllocator allocator;

  ConstExprEvaluator(const ConstExprEvaluator &) = delete;
  void operator=(const ConstExprEvaluator &) = delete;

public:
  explicit ConstExprEvaluator(SILModule &m);
  ~ConstExprEvaluator();

  llvm::BumpPtrAllocator &getAllocator() { return allocator; }

  /// Analyze the specified values to determine if they are constant values.
  /// This is done in code that is not necessarily itself a constexpr
  /// function.  The results are added to the results list which is a parallel
  /// structure to the input values.
  ///
  /// TODO: Return information about which callees were found to be
  /// constexprs, which would allow the caller to delete dead calls to them
  /// that occur after after folding them.
  void computeConstantValues(ArrayRef<SILValue> values,
                             SmallVectorImpl<SymbolicValue> &results);

  /// Try to decode the specified apply of the _allocateUninitializedArray
  /// function in the standard library.  This attempts to figure out what the
  /// resulting elements will be.  This fills in the elements result and returns
  /// true on success.
  ///
  /// If arrayInsts is non-null and if decoding succeeds, this function adds
  /// all of the instructions relevant to the definition of this array into
  /// the set.  If decoding fails, then the contents of this set is undefined.
  ///
  static bool
  decodeAllocUninitializedArray(ApplyInst *apply,
                                uint64_t numElements,
                                SmallVectorImpl<SILValue> &elements,
                                SmallPtrSet<SILInstruction*, 8> *arrayInsts);
};

} // end namespace tf
} // end namespace swift
#endif