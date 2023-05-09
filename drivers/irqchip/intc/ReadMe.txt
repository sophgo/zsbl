BM1686 Interrupt

BM1686 uses a chained interrupt design.
This platform instantiate two designware ictl and chained them together
Its topology is shown as below

                                     -------
                        IRQ0------->|       |          -----
                        IRQ1------->|       |   IRQ   |     |
                                    | ICTL0 |-------->| CPU |
                        IRQ62------>|       |         |     |
                             ------>|       |          -----
                            |        -------
                            |
             -------        | IRQ63
IRQ0------->|       |       |
IRQ1------->|       |  IRQ  |  
            | ICTL1 |-------
IRQ62------>|       |
IRQ63------>|       |
             -------
