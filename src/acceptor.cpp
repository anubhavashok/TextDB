//
//  acceptor.cpp
//  TextDB
//
//  Created by Anubhav on 3/6/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "acceptor.h"
#include "operation.h"
#include <fstream>

template <class Operation>
bool Acceptor<Operation>::promise(long long n)
{
    if (n >= minProposal) {
        mpstream.seekp(0);
        mpstream << n;
        mpstream.flush();
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
        mpstream.seekp(0);
        mpstream << n;
        mpstream.flush();
        acceptedProposal = minProposal = n;
        acceptedValue = v;
        // res.send(minProposal); ACK
        return true;
    }else {
        // NACK
        return false;
    }
}

template <class Operation>
Acceptor<Operation>::Acceptor(fs::path persistence)
{
    if (!fs::exists(persistence)) {
        fs::create_directories(persistence);
    }
    fs::path minProposalPath = persistence / "mp.var";
    if (!fs::exists(minProposalPath)) {
        ofstream temp(minProposalPath.string());
    } else {
        ifstream temp(minProposalPath.string());
        temp >> minProposal;
    }
    mpstream.open(minProposalPath.string(), ios::in | ios::out);

}