LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := javamethod.c aacrecorder.c \
	aac_rom.c \
	aacenc.c \
	aacplusenc.c \
	adj_thr.c \
	adts.c \
	band_nrg.c \
	bit_cnt.c \
	bit_sbr.c \
	bitbuffer.c \
	bitenc.c \
	block_switch.c \
	cfftn.c \
	channel_map.c \
	code_env.c \
	dyn_bits.c \
	env_bit.c \
	env_est.c \
	fram_gen.c \
	freq_sca.c \
	grp_data.c \
	hybrid.c \
	interface.c \
	invf_est.c \
	line_pe.c \
	mh_det.c \
	ms_stereo.c \
	nf_est.c \
	pre_echo_control.c\
	ps_bitenc.c \
	ps_enc.c \
	psy_configuration.c \
	psy_main.c \
	qc_main.c \
	qmf_enc.c \
	quantize.c \
	resampler.c \
	sbr_main.c \
	sbr_misc.c \
	sbr_rom.c \
	sf_estim.c \
	spreading.c \
	stat_bits.c \
	stprepro.c \
	tns.c \
	tns_param.c \
	ton_corr.c \
	tran_det.c \
	transcendent.c \
	transform.c
LOCAL_LDLIBS := -llog -lz -lm -g

LOCAL_ALLOW_UNDEFINED_SYMBOLS=false
LOCAL_MODULE := aacrecorder

include $(BUILD_SHARED_LIBRARY)
