/*
 * Copyright (c) 2020 Karsten Becker All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "SHICommunicator.h"

using SHI::Communicator;

void Communicator::accept(SHI::Visitor &visitor) {
  visitor.visit(this);
  status->accept(visitor);
}
