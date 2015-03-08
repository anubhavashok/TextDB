//
//  acceptor.cpp
//  TextDB
//
//  Created by Anubhav on 3/6/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "acceptor.h"
#include "operation.h"

template <class Operation>
bool Acceptor<Operation>::promise(long long n)
{
    if (n >= minProposal) {
        minProposal = n;
        // ACK + acceptedProposal + acceptedValue
        // res.json({res:[acceptedProposal, acceptedValue]});
        cout << "Promised OK: " << n << endl;
        return true;
    } else {
        // send NACK
        return false;
    }
}

template <class Operation>
bool Acceptor<Operation>::accept(long long n, Operation v)
{
    if (n >= minProposal) {
        cout << "Accepted: " << n << " + " << v.cmd << endl;
        acceptedProposal = minProposal = n;
        acceptedValue = v;
        // res.send(minProposal); ACK
        return true;
    }else {
        // NACK
        return false;
    }
}
