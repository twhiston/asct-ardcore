/**
 @file
 asctArdtoPD - ardcore to Pure Data object.
 Deals with serial message decode and send for the Ardcore Eurorack module
 Unlike messages from the ardcore messages sent to it have no transaction start message
 
 Thomas Whiston
 tom.whiston@gmail.com
 
 @ingroup	asctSoftware
 */

#include "PD_src/m_pd.h"

//Some character codes we use as id's when sending messages to the ardcore
#define XOFF 19
#define DIGIVAL 100
#define ANVAL 97

#define INBUFFSIZE 32


static t_class *asctArdtoPD_class;

typedef struct _asctArdtoPD {
	t_object x_obj;
	t_outlet *m_outlet1;
	t_outlet *m_outlet2;
	t_outlet *m_outlet3;
	t_outlet *m_outlet4;
	t_outlet *m_outlet5;
	t_outlet *m_outlet6;
	t_outlet *m_outlet7;
	t_outlet *m_outlet8;
    
    char serialIn[INBUFFSIZE]; //hold input data here for processing
    t_int serialIndex;// the index of where we are writing in the serialIn array
    
} t_asctArdtoPD;

void asctArdtoPD_serial_float_method(t_asctArdtoPD *x, t_floatarg n){
    //transmission start flag
    if (n == 17) {
        //when this flag is set we have a new serial message so we reset the position
        x->serialIndex = 0;
        return;
    }
    //transmission stop flag
    if(n != 19){
        //19 is the XOFF character
        x->serialIn[x->serialIndex] = n;
        //safeguard against writing outside of allocated space
        if(x->serialIndex < INBUFFSIZE-1){
            x->serialIndex++;
        } else {
            x->serialIndex = 0;
        }
        return;
    }
    if(n == 19){
        t_int temp = 0;
        //if the input is not indexed as 6 or 7 we know we must combine 2 bytes to get the output value
        if (x->serialIn[0] != 6 || x->serialIn[0] != 7)
            temp = (x->serialIn[2] & 127) + (x->serialIn[1] << 7);
        
        /**
         * now that we have the value we need to send it to the right output
         * cases 0-5 describe inputs that should be routed to the outputs
         * 6+ are special case messages, most of these go to m_outlet1 which is for serial transmission
         * 6 - respond to handshake message
         */
        switch (x->serialIn[0]) {
            case 0:
                outlet_float(x->m_outlet2, temp);
                break;
            case 1:
                outlet_float(x->m_outlet3, temp);
                break;
            case 2:
                outlet_float(x->m_outlet4, temp);
                break;
            case 3:
                outlet_float(x->m_outlet5, temp);
                break;
            case 4:
                outlet_float(x->m_outlet6, temp);
                break;
            case 5:
                outlet_float(x->m_outlet7, temp);
                break;
            case 6:
                outlet_float(x->m_outlet1, XOFF);//case 6 is the handshake, we return a byte, value doesnt matter
            case 7:
                outlet_bang(x->m_outlet8);//case 7 is a clk input so we do a bang
            default:
                break;
        }
    }
}
//DONE
void asctArdtoPD_dac_float_method(t_asctArdtoPD *x, t_floatarg n){
    int i;
    char txt[4];
    for(i = 3;i--;)
        txt[i] = 0;
    /**
     * input 1 is for the DAC out, we need to take the input and prepare a message with it
     * output message is
     * [a][VALUE]
     */
    //safeguard against the value being out of bounds
    if(n > 255)
        n = 255;
    if(n<0)
        n = 0;
    sprintf(txt, "%d", (int)n); //int to char conversion
    //now send the message
    outlet_float(x->m_outlet1, ANVAL);//a
    for (i = 0; i<3; i++) {
      //  post("txt %i : %i", i, txt[i]);
        //strip out 0's apart from in the case of the first index where 0 is allowed
        if (txt[i] != 0) {
            outlet_float(x->m_outlet1, txt[i]);
        }
        if(txt[i] == 0 && i == 0)
            outlet_float(x->m_outlet1, txt[i]);
    }
    outlet_float(x->m_outlet1, XOFF);
}

//DONE
void asctArdtoPD_digi_list_method(t_asctArdtoPD *x, t_symbol *s, int argc, t_atom *argv){
    t_atom *ap;
    t_int i;
    t_int in;
    char port[2];
    char val[2];
    
    /**
     * Inlet 2 deals with digital ons
     * message to serial is
     * [o][d][INDEX][VALUE]
     */
        //now we have to pass the index and value
        for(i = 0, ap = argv; i<argc;i++, ap++)
        {
                    in = atom_getint(ap);
                    //these returns safeguard against incorrect values
                    if (i == 0 && in > 9)
                        return;
                    
                    if (i == 1 && in > 1)
                        return;
                    
                    //turn our longs into chars
                    if (i == 0)
                        sprintf(port, "%ld", in);
            
                    if (i == 1)
                        sprintf(val, "%ld", in);
        }
        //write the outputs
        outlet_float(x->m_outlet1, DIGIVAL);//d
        outlet_float(x->m_outlet1, port[0]);
        outlet_float(x->m_outlet1, val[0]);
        outlet_float(x->m_outlet1, XOFF);
}

// Constructor of the class
void * asctArdtoPD_new(void) {
	t_asctArdtoPD *x = (t_asctArdtoPD *) pd_new(asctArdtoPD_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("floatprocess"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym("digilist"));
	x->m_outlet1 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet2 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet3 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet4 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet5 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet6 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet7 = outlet_new(&x->x_obj, gensym("float"));
	x->m_outlet8 = outlet_new(&x->x_obj, gensym("bang"));
	return (void *) x;
}

// Destroy the class
void asctArdtoPD_destroy(t_asctArdtoPD *x) {
	;
}

void asctArdtoPD_setup(void) {
	asctArdtoPD_class = class_new(gensym("asctArdtoPD"),
                                  (t_newmethod) asctArdtoPD_new, // Constructor
                                  (t_method) asctArdtoPD_destroy, // Destructor
                                  sizeof (t_asctArdtoPD),
                                  CLASS_DEFAULT,
                                  0);//Must always ends with a zero
    
	class_addfloat(asctArdtoPD_class, asctArdtoPD_serial_float_method);
    class_addmethod(asctArdtoPD_class, (t_method)asctArdtoPD_dac_float_method, gensym("floatprocess"),A_FLOAT,0);
	class_addmethod(asctArdtoPD_class, (t_method)asctArdtoPD_digi_list_method, gensym("digilist"),A_GIMME,0);
	class_sethelpsymbol(asctArdtoPD_class,gensym ("asctArdtoPD_help"));
}
